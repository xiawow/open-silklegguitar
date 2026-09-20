# 丝腿吉他 (Silk Leg Guitar) —— 逆向重建源码工程

> 对一台 ESP32 蓝牙吉他效果器固件（`丝腿吉他.bin`，1214176 字节，MD5
> `C602E9F66AE6141404AD996ACFC3A0AD`）与其 Web 蓝牙控制前端（原站
> `https://dashichang.work/_612/` 镜像）的完整逆向 + 源码重建。
> CTF 模拟实战项目 —— 安全发现见 §6。

## 1. 目录结构

```
silk-leg-guitar-src/
├── frontend/               Web 前端 (保持原始文件名, index.html 直接可跑)
│   ├── index.html          主页面 + Web Bluetooth 连接 + 歌曲加载
│   ├── _19.js              class _933 = CBinaryBuffer 的 JS 版 (两遍构建)
│   ├── _27.js              class _1170 = BLE 协议层 (25 上行 / 16 下行命令)
│   ├── _613.js             业务动作 (输出/电机/上传引擎/插件解析)
│   ├── _729.js / _731.js / _730.js   歌曲条目 / 波形 / 播放器
│   ├── _710.bin            服务器上的 OTA 固件包 (= 逆向的那份固件)
│   ├── SYMBOLS.md          ★ 前端符号映射表 (先读这个)
│   └── ...                 UI 组件 / worker / 图片 (35 个文件)
├── firmware/               ESP-IDF v5 后端骨架
│   ├── CMakeLists.txt      project(silk_leg_guitar)
│   ├── sdkconfig.defaults  BT 双模 + A2DP + GATTS
│   └── main/
│       ├── app_main.cpp        启动链 (横幅→NVS→配置→BT→音频)
│       ├── Common.h            协议常量/枚举 (全部经 JS 交叉验证)
│       ├── BinaryBuffer.*      ★★★★★ CBinaryBuffer 完整重建 (两遍构建)
│       ├── JsInterface.*       ★★★★★ BLE 命令协议层 (25 命令分发+分帧)
│       ├── UserConfigure.*     TLV 配置存取 (0xE45B901F)
│       ├── SilkLegGuitar.*     模块/输出切换 GPIO 序列 + 电机
│       ├── PickupPlugin.*      插件格式解析 (LE 头+重定位+0xCC 对齐)
│       ├── FFT.*               radix-2 实 FFT + IFFTReal (固件断言串实锤接口)
│       ├── RecordTemporarily.* 临时录音缓冲
│       ├── AutoPitch.*         ★★★★☆ 音符播放引擎 (标识符/断言/printf 全实锤;
│       │                         包络形状与 mp3 解码器 ◐)
│       ├── PluckDetector.*     ★★★★☆ 拨弦检测 (Go 四出参 / ±12 邻域峰判定 /
│       │                         65:33 起振魔数 全实锤; 窗长与历史窗 ◐)
│       └── driver/             GATT 服务 / 栈管理 / A2DP / I2S
└── docs/
    ├── 丝腿吉他_ESP32固件逆向分析报告.md    固件 RE 总报告
    └── 丝腿吉他_前端JS逆向报告.md           前端 JS RE 报告 (协议全貌)

tests/
└── test_dsp.cpp            DSP 宿主测试 (26 项检查: FFT 往返 / 拨弦检测触发
                             与音高 / 自动乐器包络逐点核对与超长告警分支)
```

## 2. 置信度状态矩阵

| 层 | 状态 | 依据 |
|---|---|---|
| BLE 包格式 (魔数 0x4E8A2F5B 大端 / 命令名 / 两遍构建) | ★★★★★ 完整 | 前端 `_19/_27.js` + 固件反汇编双向印证 |
| 25 上行命令 + 16 下行响应 | ★★★★★ 完整 | 全部命令名在固件字符串区找到 (25+16 全命中) |
| CBinaryBuffer | ★★★★★ 完整 | 6 方法逐条反汇编 + JS 逐语义对照 |
| 下行分帧 (509 上限, total/pos 重组) | ★★★★★ 完整 | `_1191` 源码级还原 |
| 分块上传 (4096 块 × 455 片) | ★★★★★ 完整 | `_8248` 源码级还原 |
| 插件文件格式 (LE 头/重定位/0xCC) | ★★★★★ 解析完整 | `_8286`; **执行** 为桩 |
| 歌曲文件格式 | ★★★★☆ | `_1210` 还原, 固件常量印证 |
| UserConfigure TLV | ★★★★★ | `_8143` + 固件常量 @0x40FE8 |
| GATT 属性表 (0x9578/0x6710) | ★★★★☆ 骨架 | 心跳 demo 改造实锤 (HRS_IDX_NB), 权限位 ◐ |

## 3. 前端运行

前端保持原始文件名与相对路径，`frontend/index.html` 可直接打开。但
**Web Bluetooth 要求 secure context**：

```text
方法 A (Chrome):
  cd frontend
  python -m http.server 8443        # 或任意静态服务器
  打开 https://localhost 不行 —— 用 chrome://flags 允许 localhost,
  或直接 http://localhost:8443 (localhost 属 secure context, 可用)

方法 B (真机调试): 用 HTTPS 域名反代 (Web Bluetooth 在非 localhost 的
  http 下不可用)。
```

连上设备后前端走 `navigator.bluetooth.requestDevice` 过滤服务 `0x9578`。
注意：**没有真实设备时 UI 可浏览但连不上**；`_710.bin` 的版本解析（`_8272`）
在纯前端即可验证（fetch 本地文件）。

## 4. 固件编译

```bash
# ESP-IDF v5 (esp32 目标, 双模 BT)
cd firmware
idf.py set-target esp32
idf.py menuconfig      # 默认 sdkconfig.defaults 已含 BT/A2DP/GATTS
idf.py build
```

- **已验证**: 全部 12 个 .cpp 通过宿主端 `g++ -fsyntax-only -Wall -Wextra`
  零警告（ESP 依赖用 `__has_include` 隔离，宿主自动走桩路径）。
- **未验证**: ESP-IDF 真机编译与运行 —— driver 层的 Bluedroid 事件接线
  （handle 缓存、注册回调、A2DP 数据回调）留 TODO，需要实机填。
- 交叉编译器/IDF 版本差异可能导致 API 细节需微调（i2s 用 IDF v5 channel API）。

## 5. 协议速查

上行 `[u32 BE 魔数 0x4E8A2F5B][string 命令名][负载]`；
下行 `[string 命令名][i32 total][i32 pos][i32 chunkLen][data]`（无魔数，
>509B 自动分帧）。纯文本类响应（ShowDebugString/FirmwareVersion/
CurrentOutput）负载是**原始 ASCII**；录音响应是 `[i32 BE 采样率][LE u16
偏置样本]` —— 协议整数大端、音频批量数据小端并存。

完整命令表 / 符号对照：
- 前端 → `frontend/SYMBOLS.md`（class _1170 全表）
- 协议细节 → `docs/丝腿吉他_前端JS逆向报告.md` 第 3 章
- 固件侧锚点（函数地址/字符串偏移）→ `docs/丝腿吉他_ESP32固件逆向分析报告.md`

## 6. 安全发现摘要（CTF 线索）

1. **OTA 无签名校验** —— 固件升级数据直接写入，无任何验证。
2. **插件魔数仅前端校验** —— `0x5D9F2A6B` 在固件二进制中不存在；
   绕过前端可下发任意 Xtensa 代码（mmap 执行路径）。
3. **蓝牙设备名未转义** 拼进 `onclick` 属性（`_27.js` 520–529 行）——
   JS 注入点，可控字段：发现设备名（30B）。
4. **PickupPluginHtml 同源 blob iframe** —— 固件→页面反向控制通道。
5. 隐藏调试按钮 `_697/_698`（Write Value / Read Characteristic）。

# 软件版权

**本项目（含全部逆向分析成果与源码重建）版权完全归本人所有。**

本软件采用 **GPLv3（GNU General Public License v3.0）** 协议开源，
完整协议文本见仓库根目录 [`LICENSE`](LICENSE)。

**例外声明**：以下主体**不获得**本软件的任何授权，不得使用、复制、修改、
分发本软件或其衍生作品：

- **赵磊**（GitHub: https://github.com/zhaoleicpp ）——此人多次侵犯本人知识产权。

除上述主体外，任何个人或组织均可按 GPLv3 协议自由使用本软件。
版权所有 © 2026 xiawow
