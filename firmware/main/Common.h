// ============================================================================
// Common.h —— 全局枚举/常量 (来源: 固件逆向 + 前端 JS 交叉验证)
// ============================================================================
#pragma once

#include <cstdint>
#include <cassert>

// 断言: 固件原版用 ESP 断言(带表达式字符串), 骨架用标准 assert
#define SLG_ASSERT(x) assert(x)

// ---------------------------------------------------------------------------
// 协议常量 —— ★ 全部经前端 _27.js (class _1170) 交叉证实
// ---------------------------------------------------------------------------
namespace slg {

// 上行包魔数 (JS _1170._8137; 固件内 LE 常量 @0x40CBC), 线上大端
constexpr uint32_t kPacketMagic        = 0x4E8A2F5B;

// GATT 特征值上限: 固件 GATT maxlen=509, JS _8135=509, SendIndicateData 断言 ≤0x1FD
constexpr int      kMaxIndicateSize    = 509;
constexpr int      kMaxWriteSize       = 512;   // 写方向 (RE 未完全确证, ◐)

// OTA / 数据上传: 4096B 块 x 455B 片 (JS _8136=455, _8248 引擎; 509-50 开销)
constexpr int      kUploadBlockSize    = 4096;
constexpr int      kUploadPieceSize    = 455;

// PickupPluginSendToCpp 单包上限 (JS: 509-50=459)
constexpr int      kPluginSendMax      = 459;

// UserConfigure TLV 条目魔数 (JS _8143; 固件 LE 常量 @0x40FE8)
constexpr uint32_t kUserConfigureMagic = 0xE45B901F;

// 自动乐器歌曲文件魔数 (JS _1210._8341; 固件 @0x40D78)
constexpr uint32_t kSongFileMagic      = 0x2B047D6A;

// 拾音器插件魔数 (JS _8286) —— 注意: 固件二进制中不存在此常量!
// (固件不校验插件魔数, 仅前端校验 —— 已知安全事实, 见逆向报告)
constexpr uint32_t kPickupPluginMagic  = 0x5D9F2A6B;

// 固件版本串 (固件 @0x4C14, 前端 _8272 解析 "Dashichang silk leg guita(version: "
constexpr const char* kFirmwareVersion = "2.0";
constexpr const char* kStartBanner     = "Dashichang silk leg guita(version: 2.0) starting...\n";

constexpr int      kSampleRate         = 16000;  // JS _1202._1869

} // namespace slg

// ---------------------------------------------------------------------------
// 枚举 —— 来源: 固件 GetOutputString/SwitchToModule 反汇编 + 字符串表
// ---------------------------------------------------------------------------
enum class EOutput : int {
    none                    = 0,
    line_out                = 1,
    speaker                 = 2,
    wireless_microphone     = 3,
    bluetooth_drive_speaker = 4,
    bluetooth_midi_keyboard = 5,
};
const char* GetOutputString(EOutput o);   // 返回 JS 侧使用的确切字符串 (固件字符串表 @0x4A08-0x4B2C)

// ◐ 模块枚举: RE 报告记 {1=pickup,2=output,3=midi_device}, 与 JS 字符串
//   ("auto_pitch"/"pickup"/"midi_device") 的对应关系存疑, 以 JS 字符串为对外标准
enum class EModule : int {
    auto_pitch  = 1,
    pickup      = 2,
    midi_device = 3,
};
const char* GetModuleString(EModule m);   // "auto_pitch" / "pickup" / "midi_device"

enum class ESource : int {   // 固件 RE: ESource{1=mono, 2=stereo}
    mono   = 1,
    stereo = 2,
};

// ◐ ADC1 电压档位枚举: 固件中枚举值未反出 (Adc1Initialize 第 2 参)
enum class EADC1VoltageRange : int {
    unknown = 0,   // TODO: 从 Adc1Dma 初始化调用点反推
};
