# 丝腿吉他 (SilkLegGuitar) ESP32 固件逆向分析报告

> 目标文件：`丝腿吉他.bin`　大小：1,214,176 字节 (0x1286E0)
> 分析环境：IDA Pro 9.1 (idalib MCP) + Xtensa 处理器模块 + esp.py 加载器
> 分析日期：2026-09-20

---

## 一、结论速览

| 项目 | 结果 |
|---|---|
| 芯片 | **ESP32** (chip_id=0x0000, Xtensa LX6) |
| 框架 | **ESP-IDF v5.4-dirty** |
| 项目名 | **silk_leg_guitar** (丝腿吉他) |
| 编译时间 | Jun 8 2026, 11:52:09 |
| 固件版本 | **2.0** |
| 入口点 | `0x400817B4` |
| 加密/混淆 | **无加密**（明文可读），但 `esp_app_desc_t.magic_word` 被改为 **`0xABCD5432`**（标准值 `0xABCEF5D3` 全固件 0 次出现）——厂商构建期改写的私有标记 |
| 符号表 | **已 strip**，但保留大量 `__FILE__` / `__PRETTY_FUNCTION__` 字符串 |
| SHA256 (镜像) | `7525ebf5fa554087361b33db02e18bee9b4b84ffff05f7ea8bf94860` ✓ 校验通过 |
| MD5 | `c602e9f66ae6141404ad996acfc3a0ad` |

**产品定性**：一款 **蓝牙智能电子乐器（智能吉他）**。ESP32 通过 ADC/I2S 采集琴弦振动，经 FFT + 拨弦检测实现自动调音 (AutoPitch)，通过 A2DP 输出音频、BLE GATT 与手机 App 通信；支持 MIDI 键盘、可插拔"拾音器插件"、OTA 升级。

---

## 二、固件结构

### 2.1 文件布局（标准 ESP-IDF 镜像格式）

```
偏移 0x000000  ┌─ 前缀头 (8 B) ─────────────────────────────
              │  magic      = 0xE9
              │  seg_count  = 6
              │  spi_mode   = 2 (DIO)
              │  spi_size   = 4MB
              │  entry_addr = 0x400817B4
偏移 0x000008  ├─ 扩展头 (16 B) ────────────────────────────
              │  chip_id    = 0x0000 (ESP32)
              │  hash_appended = 1
偏移 0x000018  ├─ 段头0 (8 B)：0x3F400020 / 0x37350
偏移 0x000020  │  seg0 数据（起始于 esp_app_desc_t）
偏移 0x037370  ├─ 段头1 (8 B) + seg1 数据
    ...       │  （段头与数据**交错排列**——esptool 标准格式，
              │    每段 8 字节头紧贴自己的数据，不是连续段表）
偏移 0x1286BC  ├─ 校验和 + 填充至 16 字节对齐
偏移 0x1286C0  ├─ SHA256 (32 B)  ← 校验通过
偏移 0x1286E0  └─ 文件结束
```

### 2.3 非标准细节：app 描述符魔数被厂商改写

- `esp_app_desc_t` 位于 DROM 段首（内存 `0x3F400020` = 文件偏移 `0x20`）
- 其 `magic_word` 本应为 `0xABCEF5D3`（`ESP_APP_DESC_MAGIC_WORD`），**本固件为 `0xABCD5432`**，且标准魔数在整个 1.2MB 固件中出现 **0 次**（排除巧合/重复结构）
- 尾部 SHA256 与（含改写魔数的）镜像体完全一致 → 魔数是**构建时**被厂商工具改写后重算哈希的，非事后篡改
- 其余字段正常：`project_name="silk_leg_guitar"`、`version="1"`、`time="11:52:09"`、`date="Jun  8 2026"`、`idf_ver="v5.4-dirty"`
- 推测用途：厂商私有打包/OTA 格式标记或防盗刷水印；标准 `esptool image_info` 仍可解析整体结构

### 2.2 六个段

| # | 加载地址 | 长度 | 类型 | 用途 |
|---|---|---|---|---|
| 0 | `0x3F400020` | 226,128 | **DROM** | 只读数据、字符串常量（含全部符号线索） |
| 1 | `0x3FF80000` | 28 | RTC_DRAM | RTC 保留内存 |
| 2 | `0x3FFBDB60` | 21,564 | **DRAM** | 已初始化数据 (.data/.bss) |
| 3 | `0x40080000` | 14,392 | **IRAM** | 中断向量/高频代码 |
| 4 | `0x400D0020` | 842,400 | **IROM** | **主代码段**（业务 + Bluedroid + IDF） |
| 5 | `0x40083838` | 109,556 | **IRAM** | 代码（含蓝牙控制器） |

> 注：`0x400D0020` – `0x400D3178` 是 **Xtensa 字面量池 (literal pool)**，IDA 将其识别为数据；真实代码自 `0x400D3178` 起。池内条目保存 DROM 字符串地址与函数指针。

---

## 三、软件架构

### 3.1 业务源码模块（从 `__FILE__` 字符串还原，共 15 个）

> `__FILE__` 字符串有两种前缀：12 个 `/./main/...` 与 3 个 `./main/driver/...`（I2S.cpp、A2DP.cpp、BluetoothManager.cpp）。

```
main/
├── SilkLegGuitar.cpp           主程序 / 模块调度
├── JsInterface.cpp             ★ App 命令分发器
├── AutoPitch.cpp               ★ 自动调音
├── PluckDetector.cpp           ★ 拨弦检测（"山峰数组"峰值检测）
├── FFT.cpp                     ★ FFT/IFFT (512/128/64/32/16/8)
├── UserConfigure.cpp           用户配置（Flash 持久化）
├── BinaryBuffer.cpp            二进制序列化缓冲
├── RecordTemporarily.cpp       临时录音（Mono/Stereo + MP3 解码）
├── PickupPlugin.cpp            ★ 拾音器插件（动态加载）
└── driver/
    ├── A2DP.cpp                ★ 蓝牙音频输出（A2DP **Source**，主动连接蓝牙音箱外放）
    ├── BluetoothManager.cpp    蓝牙管理
    ├── BluetoothCustomService.cpp  ★ BLE GATT 自定义服务（App/Chrome 通道）
    ├── I2S.cpp                 I2S 音频总线（新版 i2s_new_channel API）
    ├── Adc1Dma.cpp             ADC1 DMA 采样
    └── AdcPoll.cpp             ADC 轮询采样
```

还原出的函数签名（`__PRETTY_FUNCTION__`）：
- `void I2SThread(void*)`
- `void I2SInitialize(int, void (*)(short int (*)[2], long long unsigned int))`
- `bool A2DPInitialize(int, void (*)(unsigned char*, char*), void (*)(), void (*)(short int&, short int&))`
- `void a2dp_callback(esp_a2d_cb_event_t, esp_a2d_cb_param_t*)`
- `int BluetoothSendIndicateData(const void*, int)`
- `bool SetBluetoothAdvertiseData()`
- `void Bluetooth_gatts_profile_event_handler(esp_gatts_cb_event_t, esp_gatt_if_t, esp_ble_gatts_cb_param_t*)`

### 3.2 主要 C++ 类

| 类 | 职责 |
|---|---|
| `CJsInterface` | 命令分发（BLE → 动作） |
| `CAutoPitch` | 自动调音（音符缓存、音量包络、衰减/保持） |
| `CPluckDetector` | 拨弦检测（`S山峰数组` 峰值结构 + lambda） |
| `CFFT` | `FFTReal` / `IFFTReal`（ESP32 DSP 库） |
| `CUserConfigure` | KV 配置读写（`GetInt`/`SetString`/`SetByteArray`/`LoadFilter`） |
| `CBinaryBuffer` | 序列化（`Seek`/`SetLength`/`ReadString8`） |
| `CRecordTemporarily` | 录音缓存（Mono/Stereo） |
| `CPickupPlugin` | 插件加载（mmap 代码段/数据段） |

---

## 四、通信协议（核心成果）

### 4.1 通道

- **BLE GATT 自定义服务**（`BluetoothCustomService.cpp`）——**完整参数已还原**
  - 广播：`02 01 06`(Flags) + `03 03 78 95`(16位服务列表) + `10 09 "SILK_LEG_GUITAR"`(完整设备名)，共 24 字节原始广播包，由 `SetBluetoothAdvertiseData()` @ `0x400E2E04` 构造、`esp_ble_gap_config_adv_data_raw()` 下发
  - **Service UUID：`0x9578`**（广播列表 + GATT 属性表双重确认）
  - **Characteristic UUID：`0x6710`**，属性字节 `0x2E` = READ | WRITE_WITHOUT_RESPONSE | NOTIFY | EXTENDED，权限 READ|WRITE，maxlen=509；UUID 实体存于 **DRAM `0x3FFBE3E8`**（可写内存，初始值 0x6710，疑似支持运行时改写）
  - 属性表 `esp_gatts_attr_db_t[4]` @ DROM `0x3F424504`：主服务声明(0x2800, 值=0x9578) → 特征声明(0x2803, 属性=0x2E) → 特征值(0x6710) → CCCD(0x2902, 值=0x0002)
  - 4 个句柄 `g_custom_service_service_handle` / `char_handle` / `char_val_handle` / `char_cfg_handle` 与 4 条属性表项一一对应
  - **Chrome 分帧协议**：Chrome 浏览器（Web Bluetooth）写入的数据固定以 `[02 00]` 开头、`[80 0D]` 结尾，固件在 WRITE 事件中剥离首尾（日志 "Bluetooth skip data from chrome [0x02 0x00] always at the beginning" / "[0x80 0x0d] always at the end"）；尺寸上限 `BLUETOOTH_MAX_CHROME_TO_ESP32_SIZE` / `BLUETOOTH_MAX_ESP32_TO_CHROME_SIZE`
  - 客户端是 **Chrome 网页**（`GetPickupPluginHtml` 下发插件 UI，`PickupPluginSetJs` / `PickupPluginSendToCpp` 构成 JS↔C++ 桥）
  - 回传：`BluetoothSendIndicateData()` @ `0x400E2DA0`（未连接时 "Not connected, cannot send indicate"）
  - 事件分发：`Bluetooth_gatts_profile_event_handler()` @ `0x400E2F34`（837 字节：REG_EVT 建表 / WRITE_EVT 收命令 / READ_EVT / CONF_EVT / 连接管理）
- **A2DP Source**：向蓝牙音箱输出音频（`esp_a2d_source_register_data_callback(a2dp_audio_enc_data)`，"A2DPOnSpeakerDiscovered" 搜索音箱，采样率动态协商 `A2DP real frequency = %d(%d)`）
- **MIDI over UART**：外部 MIDI 键盘，`MidiKeyboard uart_read_bytes`

### 4.2 命令表（`CJsInterface::Drive()` @ `0x400DCBE8`，2121 字节）

命令以字符串形式由 App 下发，`Drive()` 用 **strcmp 链**分发：

| 命令 | 行为 |
|---|---|
| `GetDebugString` | → 回传 `ShowDebugString` |
| `GetCpuUsageString` | → 回传 `ShowCpuUsageString` |
| `GetMemoryUsageString` | → 回传 `ShowMemroyUsageString`（原文拼写如此） |
| `GetFirmwareVersion` | → 回传 **`"2.0"`** |
| `GetPickupPluginBaseAddress` | → 回传插件代码段/数据段基址 |
| `GetPickupPluginHtml` | → 回传插件 HTML 页面 |
| `RecordTemporarily` | → `CRecordTemporarily::RecordFuture(1=mono / 2=stereo)` |
| `GetHistoryRecord` | → `CRecordTemporarily::GetHistoryRecord(1/2)` |
| `GetUserConfigure` | → 回传 `UserConfigure` 配置块 |
| `SetOutput` | → `SwitchToModule(2)`，键 `module=output` |
| `SetAutoPitch` | → `SwitchToModule(2)`，键 `module=auto_pitch` |
| `SetPickup` | → `SwitchToModule(1)`，键 `module=pickup` |
| `SetMidiDevice` | → `SwitchToModule(3)`，键 `module=midi_device` |
| `AutoInstrumentAudioData` | 自动乐器音频数据 |
| `UpdateFirmwareData` | **OTA 固件升级** |
| `DownloadPickupPlugin` | **下载并注入拾音器插件** |
| `SetDriverVolume` | 音量（范围约 `-6/4` ~ `118/4`） |
| `DisconnectMidiDevice` / `GetMidiDeviceState` | MIDI 设备管理 |

### 4.3 数据帧格式

由 `CBinaryBuffer::ReadString8()` 解析：

```
[ 4B 长度/magic ][ 字符串: 8位长度前缀 + UTF-8 内容 ][ 参数... ]
```

### 4.4 自动乐器文件格式

`Dashichang automatic musical instrument file`（大师常自动乐器文件）：

```
magic word 校验 → 版本号 (要求 1.0) → 采样率校验 → 声道数 (要求 1=单声道)
→ 压缩模式 ("raw_short" 或 "mp3") → 目录扇区 → 音符数据
```

---

## 五、已还原的关键函数（IDB 内已命名 + 注释）

| 地址 | 名称 |
|---|---|
| `0x400DCBE8` | `CJsInterface_Drive` ★命令分发器 |
| `0x400DC834` | `CJsInterface_OnBluetoothDataCallback` |
| `0x400DC878` | `CJsInterface_SendResponse` |
| `0x400DCADC` | `CJsInterface_GetPickupPluginHtml` |
| `0x400DC754` | `CJsInterface_ShowDebugString` |
| `0x400DBA4C` | `CJsInterface_GetMemoryUsageString` |
| `0x400DC9A0` | `CJsInterface_OnModuleSwitch` |
| `0x400DC134` | `SwitchToModule` |
| `0x400DBF64` | `SwitchOutput` |
| `0x400DC0EC` | `GetOutputString` |
| `0x400DC2A8` | `SetDriverVolume` |
| `0x400DBCB4` | `FloatLerp`（FPU 线性插值） |
| `0x400DD4C8` | `CBinaryBuffer_Seek` |
| `0x400DD52C` | `CBinaryBuffer_SetLength` |
| `0x400DD7D0` | `CBinaryBuffer_ReadString8` |
| `0x400DD580` | `CBinaryBuffer_WriteString` |
| `0x400DD798` | `CBinaryBuffer_ReadHeader` |
| `0x400DD68C` | `CBinaryBuffer_Append` |
| `0x400DD908` | `CRecordTemporarily_RecordFuture` |
| `0x400DD948` | `CRecordTemporarily_GetHistoryRecord` |
| `0x400DF97C` | `CUserConfigure_Load` |
| `0x400DF9A0` | `CUserConfigure_SetStringWithKey` |
| `0x400DE62C` | `CUserConfigure_SetModuleKey` |
| `0x400DE678` | `CUserConfigure_ApplyOutput` |
| `0x400E2DA0` | `BluetoothSendIndicateData` ★BLE 回包发送 |
| `0x400E2E04` | `SetBluetoothAdvertiseData` ★构造 24 字节原始广播包 |
| `0x400E2EA8` | `StartBluetoothAdvertising` |
| `0x400E2F34` | `Bluetooth_gatts_profile_event_handler` ★GATT 事件分发（837B，含 Chrome 分帧剥离） |

---

## 六、安全分析

### 6.1 高危：OTA 与插件下载无签名校验

```
"Error! esp_ota_get_next_update_partition failed!"
"UpdateOnlineStart esp_partition_t address=%lu, size=%lu, erase_size=%lu label=%s"
"Error! esp_ota_begin failed: %d"
```

- OTA 走标准 `esp_ota_begin` / `esp_ota_write`，**未见任何签名、RSA/ECDSA 校验、加密逻辑**
- `DownloadPickupPlugin` → `PickupPluginSetJs` 直接下载并注入脚本
- `CPickupPlugin` 通过 `spi_flash_mmap` 映射插件代码段并 **直接执行**
- 未启用 Secure Boot（镜像 SHA256 仅作完整性校验，非签名）

**风险**：攻击者只要能连接该 BLE 设备（通常无需配对/认证），即可：
1. 刷入任意固件（`UpdateFirmwareData`）→ 完全控制设备
2. 下载恶意拾音器插件 → 在设备上执行任意代码

### 6.2 中危：BLE 无认证暴露调试接口

- `GetDebugString` / `GetCpuUsageString` / `GetMemoryUsageString` 泄露内部状态
- `GetPickupPluginBaseAddress` 泄露内存布局（绕过 ASLR）
- `GetUserConfigure` 可读取全部用户配置

### 6.3 低危：明文存储

- 用户配置以明文 KV 形式存于 Flash（`CUserConfigure` 无加密）
- 未启用 Flash Encryption

---

## 七、技术要点备注

1. **Hex-Rays 不可用**：Xtensa 架构无反编译器，本报告基于汇编 + 字符串交叉引用还原。
2. **literal pool 干扰**：`0x400D0020`–`0x400D3178` 为字面量池，导致 IDA 漏分析部分函数（如 `0x400DC2E3`–`0x400DC754` 区间）。可用 `define_func` 手工补全。
3. **字符串引用方式**：业务代码通过 `l32r` 从 literal pool 加载 DROM 地址，IDA 的 `string_ref_count` 常为 0，需用 `xrefs_to(池条目地址)` 二次跳转定位函数。
4. **assert 被移除**：`CAutoPitch` / `CFFT` / `CPluckDetector` 的纯 assert 字符串残留但无引用（`NDEBUG` 编译），需通过调用链反推函数位置。
5. **结构体模式扫描**：DROM 中的 `esp_gatts_attr_db_t`（24 字节/项，uuid_len + uuid_p 指向段内）可用 Python 直接从二进制扫出属性表，配合广播 AD 结构（`02 01 06` / `03 03 XX XX` / `10 09 名称`）交叉验证 UUID——比在 IDA 里翻 xref 快得多。
6. **池条目后缀指针**：编译器会引用字符串的**后缀**（如 `0x3F40671B` 指向 `"/./main/driver/BluetoothCustomService.cpp"` 的第 15 字节起），搜 `__FILE__` 时两种路径前缀（`/./` 与 `./`）都要匹配，否则会漏文件。

---

## 八、建议的后续工作

1. **补全函数识别**：在 `0x400DC2E3`–`0x400DC754` 等池间隔区手工创建函数（本轮已在蓝牙模块补全 4 个）
2. ~~提取 GATT UUID~~ **✅ 已完成**：Service `0x9578` / Char `0x6710` / 广播名 `SILK_LEG_GUITAR` / Chrome 分帧 `[02 00]…[80 0D]`
3. **还原 AutoPitch 算法**：`CAutoPitch::Add` 是核心，需结合 FFT 常量（512/128）反推音高检测参数
4. **协议实测**：用 Chrome Web Bluetooth / nRF Connect 连接设备，验证命令表、UUID 与分帧
5. **插件格式分析**：`CPickupPlugin` 的代码段/数据段结构 + JS 注入机制
6. **0xABCD5432 魔数溯源**：若能拿到官方升级包，对比确认是否为厂商 OTA 容器格式

---

## 附：复现步骤

```bash
# IDA 加载（必须指定 esp32 格式，否则走二进制加载器失败）
idat.exe -A -T"esp32" -S<script.py> firmware.bin

# 或本环境：通过 idalib MCP
idb_open(input_path=<firmware>, mode="force_headless", run_auto_analysis=true)
```

> ⚠️ 注意：不要在原文件目录留下损坏的 `.i64`（批处理模式若未指定 `-T"esp32"` 会生成只有 1 个函数的坏库，后续打开会复用它）。建议在独立工作目录分析。
