# 前端符号映射表（SYMBOLS.md）

> 原站所有标识符是纯数字编号（作者的 ID 系统），**没有**字符串加密/控制流混淆。
> 本表给出核心符号的真实身份。JS 文件保持原始文件名（保证 index.html 直接可跑），
> 可读性靠本表 + 各文件头部注释。

## 一、文件身份

| 原文件 | 身份 | 固件对应 |
|---|---|---|
| `_19.js` | **class `_933` = CBinaryBuffer（JS 版）** | `main/BinaryBuffer.cpp` |
| `_27.js` | **class `_1170` = BLE 协议层** | `main/JsInterface.cpp` + `driver/BluetoothManager.cpp` |
| `_613.js` | 业务动作（输出/电机/上传引擎/插件解析） | `main/SilkLegGuitar.cpp` / `PickupPlugin.cpp` |
| `_716.js` | UI 锁框架 `class _8197` | — |
| `_729.js` | **class `_1210` = 歌曲条目 + 格式常量** | 自动乐器数据结构 |
| `_731.js` | class `_1202` = 波形类（16kHz） | `main/FFT.cpp` 相关 |
| `_730.js` | class `_8328` = 播放器 | — |
| `_262.js` | 工具库 `class _878`（Misc） | — |
| `_22.js` / `_267.js` | 复选框 / 进度条组件 | — |
| `_10/_11.js` | MP3/WAV 解码调度 | esp-libhelix-mp3 对端 |
| `languages.js` | 翻译表（L/F/P/A/R） | — |
| `index.html` | 主页面 + Web Bluetooth 连接 + 歌曲加载 | — |
| `_718.htm` / `_722.html` | 顶栏动画 / LOADING 动画 | — |
| `_710.bin` | 固件 OTA 包（= 逆向的 `丝腿吉他.bin`，MD5 C602E9F6...） | — |

## 二、class `_933`（CBinaryBuffer）方法对照

| JS | 语义 | 固件对应 |
|---|---|---|
| `_1077` | 干跑标志（只计数不写入） | `+0 flag` 字段 |
| `position` | 游标 | `+0xC file_position` |
| `_939` / `_1078` | ArrayBuffer / DataView | `+4 buffer` / `+8 length` |
| `_935()` / `_936()` | 开始干跑 / 按尺寸分配并真写 | flag 置位 / 落地 |
| `_1082(x)` | 追加 u32（**大端**） | `AppendU32` |
| `_1085(x)` | 追加 i32（大端） | 同上（有符号） |
| `_1079/_1080/_1081` | i8 / i16 / u16 | — |
| `_1086/_1087` | i64 / f64（f64 小端） | — |
| `_1088(s)` | 追加字符串：i32 长度 + ASCII 字节 | `AppendString` |
| `_937(a)` | 追加 i32 数组（每元素 4B） | — |
| `_1092(buf)` | 追加原始字节 | `AppendBuffer` |
| `_1089(b)` | 追加另一个 `_933` 的全部内容 | — |
| `_1105()` | 读字符串（i32 长度 + ASCII） | `ReadString8` |
| `_1104()` | 读 UTF32 字符串（i32 个数 + 每字符 u32） | 歌曲元数据用 |
| `_1100()/_1101()` | 读 i32 / u32（大端） | `ReadU32` |
| `_1096()/_1097()/_1098()/_1099()` | i8/i16/u16/bool | — |
| `_1102()/_1103()` | i64 / f64(LE) | — |
| `_1106(dst,n)` | 拷出 n 字节到新 `_933` | `ReadBuffer` |
| `_1108(n)` | 切片 n 字节 | — |
| `_1110(n)` | 重置为 n 字节空缓冲 | `SetLength` |
| `_1094(buf)` | 包装已有 ArrayBuffer | `Attach` |
| `_1116()` | 总长 | `GetLength` |
| `_1025()` | 释放 | — |
| `_1123(dst,src,n)` | 静态：拷 n 字节并推进双方游标 | — |

## 三、class `_1170`（BLE 协议层）

### 常量
| JS | 值 | 含义 |
|---|---|---|
| `_8134` / `_8135` | 509 / 509 | 指示上限 / 收包重组上限（=GATT maxlen） |
| `_8136` | 455 | OTA 分块负载（509−50 开销） |
| `_8137` | `0x4E8A2F5B` | **包魔数**（上行包第一个 u32，大端；固件常量@0x40CBC） |
| `_8138` / `_8139` | — | 收包重组缓冲 / PickupPluginHtml 重组缓冲 |

### 发送（→ 固件）
| JS 方法 | 命令 | 负载 |
|---|---|---|
| `_8161` | GetDebugString | — |
| `_8162` / `_8163` | GetCpuUsageString / GetMemoryUsageString | — |
| `_8166` | GetFirmwareVersion | — |
| `_8167` | GetUserConfigure | — |
| `_8170` | GetPickupPluginBaseAddress | — |
| `_8171` | GetPickupPluginHtml | — |
| `_8168` / `_8169` | GetBluetoothDriveSpeakState / GetMidiDeviceState | — |
| `_8164(t)` / `_8165(t)` | RecordTemporarily / GetHistoryRecord | string "input"/"output" |
| `_8172(o)` | SetOutput | string（5 个 EOutput 名） |
| `_8173/8174/8175` | SetAutoPitch / SetPickup / SetMidiDevice | — |
| `_8176` / `_8177` | BluetoothDriveSpeakerDisconnect / Search | — |
| `_8179(mac,name)` | BluetoothDriveSpeakerConnect | 6B MAC + 30B 名字(29+NUL) |
| `_8178` | DisconnectMidiDevice | — |
| `_8187(m,d,ms)` | MountMotor | string "1"/"2" + string "Up"/"Down" + u32 毫秒 |
| `_8191` | UpdateFirmwareDataCanceled | — |
| `_8192(bytes)` | PickupPluginSendToCpp | 原始字节 ≤459 |
| `_613.js:_714/_8281/_8283` | AutoInstrumentAudioData / UpdateFirmwareData / DownloadPickupPlugin | 分块：块序号+总块数+片序号+片大小+本块片数+数据 |

### 接收（← 固件，`_1191` 分发）
| 命令 | 负载 | 处理 |
|---|---|---|
| ShowDebugString / ShowCpuUsageString / ShowMemroyUsageString | ASCII 原文 | 显示在调试页签（"Memroy" 为固件原生拼写） |
| FirmwareVersion | ASCII | `_8145` 比对 `_710.bin` 版本决定升级按钮 |
| UserConfigure | TLV：`u32 0xE45B901F + string 键 + u32 长度 + 值` | `_8143` 解析 module/output |
| PickupPluginBaseAddress | u32 代码段基址 + u32 数据段基址 | 存 `_8146/_8147` 供插件重定位 |
| PickupPluginHtml | u32 offset + u32 total + 数据 | `_8149` 渲染进 iframe（同源 blob） |
| BluetoothDriveSpeakState | string 状态（`not_initialzied`/`free`/`searching_speaker`/`connecting`/`connected`+MAC+名） | `_8150` |
| MidiDeviceState | u8 启用 + u8 已连 + string 名 | `_8151` |
| BluetoothDriveSpeakDiscovered | 6B MAC + 30B 名 | `_8152` 追加列表（**onclick 注入点**） |
| CurrentOutput | string | `_8153` 状态灯 |
| RecordTemporarilyMonoData / StereoData | u32 采样率 + u16 样本(=s16+0x8000) | `_8142` 转 WAV 下载 |
| UpdateFirmwareData_Success / _Failed | — | 提示重启 / 报错 |
| PickupPluginSetJs | JS 原始字节 | 调插件 iframe `_8155` |

## 四、其他关键符号

| JS | 身份 |
|---|---|
| `_8388/_8389/_8390`（index.html） | `SILK_LEG_GUITAR` / svc `0x9578` / char `0x6710` |
| `_8337` | 连接设备主流程（requestDevice→gatt→notify） |
| `_1170._1191` | 收包重组+命令分发 |
| `_8248`（_613.js） | 分块上传引擎（4096 块 × 455 片） |
| `_8286`（_613.js） | 拾音器插件解析/重组（魔数 `0x5D9F2A6B`，**头字段小端**） |
| `_8272`（_613.js） | fetch `_710.bin` 并解析版本串 |
| `_1210._8341` | 歌曲文件魔数 `0x2B047D6A` |
| `_1210._8259/_8357` | 4096 / 24576（头区布局） |
| `_8328._8329` | 空格键播放下一个音符 |
| `_878._1874` | PCM→WAV 封装 |
| `_878._1824/_1838` | base64 解码/编码 |
| `L/F/P/A/R` | 翻译/恒等/恒等/恒等/未实现 |
| `_697/_698` | 隐藏调试按钮 Write Value / Read Characteristic |
