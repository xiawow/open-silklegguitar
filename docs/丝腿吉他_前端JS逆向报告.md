# 丝腿吉他 Web 前端 JS 逆向研究报告

> 挑战入口：`https://dashichang.work/_612/`
> 关联成果：`reversed/reconstructed/`（15 个 C++ 源文件重建）、`reversed/丝腿吉他_ESP32固件逆向分析报告.md`
> 本报告聚焦前端 JS，并与已逆向的固件二进制逐项交叉验证。

---

## 一、结论速览

1. **前端 JS 就是固件 BLE 协议的另一半**。`_27.js` 中的 `_1170` 类完整实现了固件 `JsInterface.cpp` 的对端协议；全部命令名、魔数、负载格式已从 JS 挖出，并**逐条在固件二进制中得到验证**（25 发 + 16 收全部命中）。
2. **JS 版 `CBinaryBuffer`（class `_933`）与固件 `CBinaryBuffer` 逐字段同源**，彻底解释了协议的大端序和"两遍构建"机制。
3. **服务器上的"最新固件" `_710.bin` 与本地 `丝腿吉他.bin` MD5 完全相同**（`C602E9F66AE6141404AD996ACFC3A0AD`，1214176 字节）——不存在第二份固件，站点 OTA 通道下发的就是手头这份。
4. 三个私有文件格式（自动乐器歌曲、拾音器插件、UserConfigure）的完整格式已从 JS 还原。
5. 若干**安全发现**（模拟实战要点）：OTA 无签名校验、插件魔数仅前端校验、蓝牙设备名注入 onclick 属性、固件可控的 iframe HTML/JS 注入、隐藏调试按钮。

---

## 二、JS 文件清单与角色划分

| 文件 | 大小 | 角色 | 对应固件 |
|---|---|---|---|
| `_262.js` | 18KB | 基础工具库：UI 使能/禁用（`_878`）、base64 编解码（`_1824`/`_1838`）、WAV 封装（`_1874`）、hexdump（`_1117`） | — |
| `_19.js` | 5.7KB | **class `_933` = JS 版 CBinaryBuffer** | `main/BinaryBuffer.cpp` |
| `_27.js` | 13KB | **class `_1170` = BLE 协议层**（收发、命令封装、分帧重组） | `main/JsInterface.cpp` + `driver/BluetoothManager.cpp` |
| `_613.js` | 17KB | 业务动作：输出切换、电机安装、固件/歌曲/插件下载上传（`_8248` 分块引擎）、插件格式解析（`_8286`） | `SilkLegGuitar.cpp` / `PickupPlugin.cpp` |
| `_716.js` | 4.4KB | UI 锁框架（`_8197`，四种控件类型） | — |
| `_729.js` | 320B | **class `_1210` = 歌曲条目 + 文件格式常量**（魔数 `0x2B047D6A`） | 自动乐器数据结构 |
| `_731.js` | 8.8KB | class `_1202` = 音频波形类（16kHz，播放/滤波/重采样） | `main/FFT.cpp` 相关 |
| `_730.js` | 3.7KB | class `_8328` = 播放器（空格键逐音符播放，ScriptProcessor 输出） | — |
| `_22.js` / `_267.js` | 2.5K/3.7K | 复选框 / 进度条 UI 组件 | — |
| `_10/_11.js` | 1.5KB | MP3 解码调度（Worker `_979` + WAV `PCMData` 解码 `_986`） | `esp-libhelix-mp3` 的对端 |
| `languages.js` | 23KB | 翻译表（`L/F/A/P/R` 均为恒等或查表，无混淆） | — |
| `index.html` | 44KB | 主页面：8 个页签、Web Bluetooth 连接逻辑、歌曲文件加载校验 | — |
| `_718.htm` / `_722.html` | 10K/6K | 纯装饰：顶栏粒子动画 / LOADING 动画 | — |
| `_9759.html` | 278B | **404**（MIDI 键盘测试页已被移除） | — |

命名规律：全站标识符是纯数字编号（`_933`、`_1170`、`_8137`…），无字符串混淆；`A()/F()/P()` 三个"加密"函数实为恒等函数（作者注释：A for action, F for Fix, P for path）。

---

## 三、BLE 协议完整还原（JS 侧实证）

### 3.1 连接参数（与固件 RE 完全一致）

```js
const _8388 = "SILK_LEG_GUITAR";   // 设备名
const _8389 = 0x9578;              // Service UUID（optionalServices）
const _8390 = 0x6710;              // Characteristic UUID（写 + notify）
```

### 3.2 包构造：两遍构建（解开了固件 RE 的"flag 字段"之谜）

`class _933` 的关键语义：

```js
_935() { this.position=0; this._1077=true;  }  // 干跑模式：只推进 position，不写入
_936() { this._939=new ArrayBuffer(this.position); this.position=0;
         this._1077=false; /* 建 DataView */ }  // 按干跑出的尺寸分配真缓冲，再真写一遍
```

所有发送函数都是同一个模板：

```js
let buffer = new _933();
for (let i = 0; i < 2; i++) {
    if (i == 0) buffer._935();   // 第一遍：量尺寸
    else        buffer._936();   // 第二遍：真写入
    buffer._1082(0x4E8A2F5B);    // 包魔数（大端）
    buffer._1088("命令名");       // i32 长度前缀 + ASCII 字节
    // ...命令负载
}
await _8158.writeValue(buffer._939);   // 一次性写特征值
```

**重要修正**：此前固件 RE 报告推断 `[02 00]`/`[80 0D]` 分帧是"Chrome Web Bluetooth 添加"的——**JS 源码证明本页面不添加任何帧标记**，Web Bluetooth 也是原样写字节。该观察（固件日志中的帧头/帧尾）来源需重新核查（可能是厂商自用客户端的旧版协议或另有封装层），不应归因给 Chrome。

### 3.3 线上格式

```
请求（App → 固件）：
  +0x00  u32 BE  0x4E8A2F5B          包魔数（JS _8137）
  +0x04  i32 BE  命令名长度
  +0x08  ...    命令名 ASCII
  +....  ...    命令负载（见 3.4 表）

响应（固件 → App）经 _1191 重组：
  i32 BE 命令名长度 + 命令名
  u32 BE total（本条消息总长）
  u32 BE position（当前累计偏移）
  u32 BE chunk_len
  ...chunk 数据（position==0 时按 total 重置重组缓冲）
```

MTU 常量：硬上限 **509**（`_8135`，与固件 GATT maxlen=509 一致）；OTA 分块负载 **455**（`_8136`，509−50 开销）；`PickupPluginSendToCpp` 单包上限 459（509−50）。

### 3.4 完整命令表（已在固件二进制中逐条验证）

**上行 25 条**（App → 固件；地址为固件字符串偏移）：

| 命令 | 负载 | 固件@ |
|---|---|---|
| GetDebugString | 无 | 0x5068 |
| GetCpuUsageString | 无 | 0x50E0 |
| GetMemoryUsageString | 无 | 0x5108 |
| GetFirmwareVersion | 无 | 0x5138 |
| GetUserConfigure | 无 | 0x51D8 |
| GetPickupPluginBaseAddress | 无 | 0x5160 |
| GetPickupPluginHtml | 无 | 0x5194 |
| GetBluetoothDriveSpeakState | 无 | 0x5300 |
| GetMidiDeviceState | 无 | 0x531C |
| RecordTemporarily | string: "input"/"output" | 0x51A8 |
| GetHistoryRecord | string: "input"/"output" | 0x51C4 |
| SetOutput | string: 5 个 EOutput 名 | 0x51FC |
| SetAutoPitch / SetPickup / SetMidiDevice | 无 | 0x5208/0x5224/0x5230 |
| DisconnectMidiDevice | 无 | 0x536C |
| BluetoothDriveSpeakerDisconnect / Search | 无 | 0x5330/0x5350 |
| BluetoothDriveSpeakerConnect | 6B MAC + 30B 名字（29+NUL） | 0x5384 |
| MountMotor | string "1"/"2" + string "Up"/"Down" + u32 毫秒 | 0x53BC |
| UpdateFirmwareDataCanceled | 无 | 0x52E4 |
| PickupPluginSendToCpp | 原始字节（≤459） | 0x53A4 |
| UpdateFirmwareData / AutoInstrumentAudioData / DownloadPickupPlugin | 分块（见 3.5） | 0x5258/0x5240/0x526C |

**下行 16 条**（固件 → App）：

| 响应 | 负载 |
|---|---|
| ShowDebugString / ShowCpuUsageString / ShowMemroyUsageString（固件原生拼写错误 "Memroy"） | ASCII 文本 |
| FirmwareVersion | ASCII（"2.0"） |
| UserConfigure | TLV 序列：`u32 0xE45B901F + string 键名 + u32 长度 + 值`；键：`module`、`output` |
| PickupPluginBaseAddress | u32 代码段基址 + u32 数据段基址 |
| PickupPluginHtml | u32 offset + u32 total + 数据（offset==0 重置重组，完成后渲染进 iframe） |
| BluetoothDriveSpeakState | string 状态：`not_initialzied`（原生拼写错误）/`free`/`searching_speaker`/`connecting`/`connected`（+6B MAC +30B 名字） |
| MidiDeviceState | u8 启用 + u8 已连 + string 名 |
| BluetoothDriveSpeakDiscovered | 6B MAC + 30B 名字 |
| CurrentOutput | string（5 个 EOutput 名之一） |
| RecordTemporarilyMonoData / StereoData | u32 采样率 + s16 样本（u16−0x8000 还原），前端转 WAV 下载 |
| UpdateFirmwareData_Success / _Failed | 无 |
| PickupPluginSetJs | 原始 JS 字节 → 注入插件 iframe 调用 `_8155(bytes)` |

### 3.5 分块上传格式（固件升级 / 歌曲下载 / 插件下载共用引擎 `_8248`）

外层把文件切成 4096B 块，块内再切 455B 片：

```
u32 0x4E8A2F5B
string 命令名（"UpdateFirmwareData" / "AutoInstrumentAudioData" / "DownloadPickupPlugin"）
u32 块序号（4KB 单位）
u32 总块数
u32 片序号（455B 单位）
u32 片大小（455）
u32 本块片数
...片数据（≤455B）
```

固件升级上限 2MB；插件上限 800B×1024；歌曲无上限。升级完成固件回 `UpdateFirmwareData_Success` 后重启，前端提示"固件升级成功，已经重启，请重新连接..."。

---

## 四、三个私有文件格式还原

### 4.1 自动乐器歌曲文件（`_715`/`_8339`，固件类 `_1210`）

```
+0x0000  u32 BE 魔数 0x2B047D6A          （"魔字不对"报错）
+0x0004  u32 BE 版本主 + u32 BE 版本次    （必须 ≤ 1.0）
+0x000C  u32 BE 采样率标记（==16000）+ u32 BE 声道数（==1）
+0x0014  string 压缩方式："raw_short" 或 "mp3"   （每字符 u32！非字节串）
         string 原始文件名 / 作者 / 组织 / 说明   （4 条，同样每字符 u32）
         u32 歌曲数
+0x1000  （_8259=4096，头部区到此为止）歌曲条目：
           u32 编号
           u32 标志（bit31=_1213、bit30=_8355，低 30 位=参数）
           u32 音频1偏移 / u32 音频1大小 / u32 音频2偏移 / u32 音频2大小
+0x7000  （4096+24576=28672，_8356）音频数据区（偏移相对此处）
```

常量：单音符缓冲 64000 样本（4 秒@16kHz）、收尾淡出 3200 样本、最大 2806×4096（≈11.5MB）。
固件验证：魔数 0x2B047D6A 在固件中找到（0x40D78 BE / 0x40F34 LE）；`raw_short`/`mp3` 字符串在 0x5858/0x56AE 区。

### 4.2 拾音器插件文件（`_8286`，对应固件 `PickupPlugin.cpp` mmap 加载）

**头部整数字段为小端**（先按大端读再 bswap，净效果=LE），是给 Xtensa（LE）用的原生代码：

```
u32 魔数 0x5D9F2A6B          ← 仅前端校验！固件内不存在此常量
u32 ×5 （版本/入口等字段）
u32 符号表偏移 / 大小
u32 代码段偏移 / 大小
u32 重定位条数 n1 + n1 × u32 偏移   （代码字 += 数据段基址）
u32 重定位条数 n2 + n2 × u32 偏移   （代码字 += 代码段基址）
u32 数据段偏移 / 大小
u32 字段 + 两组数据段重定位（+= 数据段基址 / 代码段基址）
代码段原始字节、数据段原始字节、符号表
```

前端重组：重定位加基址 → 代码段向前对齐到 64KB 边界（**0xCC 填充**，int3）→ 回填三个最终地址 → 得到下发镜像。代码/数据段各限 128KB。
固件侧先发 `GetPickupPluginBaseAddress` 取代码/数据基址（对应 `PickupPlugin.cpp` 的 `m_code_segment__fix_address` 等成员），再 `DownloadPickupPlugin` 分块下发。

### 4.3 UserConfigure（Flash 配置，`_8143` 解析）

条目序列，每条：`u32 0xE45B901F + string 键名 + u32 长度 + 原始值`。
已知键：`module`（"auto_pitch"/"pickup"/"midi_device"）、`output`（5 个 EOutput 名，缺省 `line_out`）。
固件验证：0xE45B901F（LE 常量）@0x40FE8 —— 固件在构造响应时使用同一魔数。

---

## 五、JS ↔ 固件 符号对照表

| JS 符号 | 身份 | 固件对应 |
|---|---|---|
| `_933` | CBinaryBuffer | `CBinaryBuffer`（+0 flag↔`_1077` 干跑标志、position↔游标、`_1082` setUint32 无 endian 参数=**大端**） |
| `_1170` | BLE 协议层 | `JsInterface::Drive()` + `BluetoothManager` |
| `_1170._8137` | 包魔数 0x4E8A2F5B | 0x40CBC（LE 常量） |
| `_8138/_8139` | 收包重组缓冲 | 固件侧对应接收缓冲 |
| `_1210` | 歌曲条目类 | 自动乐器数据结构 |
| `_1202` | 波形类（16kHz） | RecordTemporarily/FFT 相关 |
| `_1210._8341` | 歌曲魔数 0x2B047D6A | 0x40D78 |
| `SILK_LEG_GUITAR / 0x9578 / 0x6710` | BLE 三元组 | 广播包 + GATT 属性表（已从固件出土，完全一致） |
| 命令字符串区 | Drive() 分发表 | 0x4F44–0x53D4 连续字符串区 |
| `Dashichang silk leg guita(version: ` | 版本串解析 | 固件 @0x4C14，实际值 **2.0** |

---

## 六、安全发现（模拟实战）

1. **OTA 固件下发无签名/无加密校验**：BLE 特征值可写（WRITE_WO_RSP），任何 Web Bluetooth 客户端都能发 `UpdateFirmwareData` 重刷 2MB 固件，固件回 `_Success` 后重启——设备完全受攻击者固件控制（需要物理接近 + 浏览器授权弹窗）。
2. **插件魔数只在前端校验**：固件二进制中不存在 0x5D9F2A6B，`DownloadPickupPlugin` 的负载直接进 mmap 执行路径；绕过前端自编客户端可下发任意 Xtensa 代码（配合无签名 OTA = 同一攻击面）。
3. **蓝牙设备名注入（onclick 属性）**：`_8152` 把搜索到的音箱名字/MAC 直接拼进 `onclick="..."` 属性字符串（`A()` 是恒等函数，无转义）。恶意蓝牙设备名形如 `');alert(1)//` 可在点击时执行任意 JS（需要诱导固件/环境返回受控名字，模拟场景中成立）。
4. **固件可控的 iframe HTML（同源 blob）**：`PickupPluginHtml` 响应内容以 `URL.createObjectURL(new Blob([html], {type:"text/html"}))` 塞进 `_645` iframe——blob 继承页面源，固件返回的 HTML/JS 与主页面**同源**，可调用 `_1170._8157()` 等页面函数（固件→页面反向控制通道）。`PickupPluginSetJs` 更是直接把固件下发的 JS 字节交给插件 iframe 执行。
5. **隐藏调试通道**：页面右下角有 Write Value/Read Characteristic 按钮（`_697/_698`，默认禁用，连接后被 `_8237` 解锁）；`GetDebugString/GetCpuUsageString/GetMemoryUsageString` 三条调试命令对普通用户不可见但固件常驻。
6. **两个保留的原生拼写错误**（可作指纹）：`ShowMemroyUsageString`（Memory）、`not_initialzied`（initialized）——固件与前端一致，可用来确认客户端与固件的配对版本。

---

## 七、对既有固件 RE 结论的修订

| 原结论 | 修订 |
|---|---|
| "`[02 00]`/`[80 0D]` 分帧由 Chrome Web Bluetooth 添加" | **撤回**。JS 源码证明应用层不添加帧标记，Web Bluetooth 原样写字节。固件日志中观察到的该帧头/帧尾来自其他客户端（厂商自用工具或旧版协议），归因需重新核查固件 WRITE_EVT 处理逻辑 |
| CBinaryBuffer `+0 flag` 字段含义存疑 | 已确证：= JS `_1077` "干跑"标志（量尺寸/真写两遍构建），JS 与固件逐语义同源 |
| Drive() 命令表不完整（17 条） | 补全为 **25 发 + 16 收**，负载格式全部还原，字符串地址逐条锚定 |
| JsInterface.cpp 协议小结（大端 + 509 上限） | 维持成立，且补上包魔数 0x4E8A2F5B、分块上传 455/4096 两级结构 |

---

## 八、附件与产物

- `ctf/_612/`：全部 15 个前端文件原始副本（含 `_710.bin` 服务器固件）
- `ctf/scan_fw.py` / `ctf/scan_fw_result.txt`：JS↔固件交叉验证脚本与结果（全命中）
- `ctf/_612/index.html` 等：`_718.htm`、`_722.html` 为纯装饰动画，无隐藏逻辑；`_9759.html` 404
- `reversed/reconstructed/main/JsInterface.cpp`：已按本报告补全命令表（见文件头注释）

## 九、下一步建议

1. 用本报告协议写一个独立 Web Bluetooth 客户端（Node/Chrome 控制台即可），对真机复现 `GetDebugString`/`GetFirmwareVersion`（预期回 "2.0"）。
2. 若要继续还原固件：以 0x4F44–0x53D4 字符串区为锚点反查 `Drive()` 的 strcmp 分发跳转表，逐命令还原处理体（正好补齐 JsInterface.cpp 的 ★ 部位）。
3. 固件侧 WRITE_EVT 帧头剥离逻辑重新核查，弄清 `[02 00]/[80 0D]` 的真实来源。
