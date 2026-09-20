// ============================================================================
// SilkLegGuitar.cpp —— 整机业务层实现
// ----------------------------------------------------------------------------
// GPIO 序列来源: 固件反汇编 (SwitchToModule GPIO40 / SwitchOutput GPIO
// 40/2/32/14)。原始时序细节 (拉低/拉高宽度、保持时间) RE 未完整反出, 骨架
// 以"置有效电平+延时+恢复"占位并标 TODO; ESP 依赖用 __has_include 隔离,
// 宿主端可单测字符串映射。
// ============================================================================

#include "SilkLegGuitar.h"
#include "UserConfigure.h"

#include <cstdio>
#include <cstring>

// 内部解析 (字符串 → 枚举), 定义在下方
static bool ParseOutputString(const char* name, EOutput* out);
static bool ParseModuleString(const char* name, EModule* out);

// ---------------------------------------------------------------------------
// 字符串映射 (固件字符串表 @0x4A08-0x4B2C; 前端 SetOutput/_8143 依赖原文)
// ---------------------------------------------------------------------------
const char* GetOutputString(EOutput o)
{
    switch (o) {
        case EOutput::none:                    return "none";
        case EOutput::line_out:                return "line_out";
        case EOutput::speaker:                 return "speaker";
        case EOutput::wireless_microphone:     return "wireless_microphone";
        case EOutput::bluetooth_drive_speaker: return "bluetooth_drive_speaker";
        case EOutput::bluetooth_midi_keyboard: return "bluetooth_midi_keyboard";
    }
    return "none";
}

const char* GetModuleString(EModule m)
{
    switch (m) {
        case EModule::auto_pitch:  return "auto_pitch";
        case EModule::pickup:      return "pickup";
        case EModule::midi_device: return "midi_device";
    }
    return "pickup";
}

// ---------------------------------------------------------------------------
// 状态
// ---------------------------------------------------------------------------
namespace {
EModule s_module = EModule::pickup;   // 缺省 ◐ (与 UserConfigure 缺省一致)
EOutput s_output = EOutput::line_out; // JS _8143 实证缺省
}

void CSilkLegGuitar::Init()
{
    char buf[32];
    if (CUserConfigure::GetString("module", buf, sizeof(buf))) {
        EModule m;
        if (ParseModuleString(buf, &m))
            s_module = m;
    }
    if (CUserConfigure::GetString("output", buf, sizeof(buf))) {
        EOutput o;
        if (ParseOutputString(buf, &o))
            s_output = o;
    }
    // TODO: GPIO 引脚初始化 (40/2/32/14 + 电机脚)
}

// 字符串 → 枚举 (供 Init 用; JsInterface.cpp 另有同名逻辑在匿名命名空间)
static bool ParseOutputString(const char* name, EOutput* out)
{
    for (int i = 0; i <= 5; ++i) {
        EOutput o = (EOutput)i;
        if (strcmp(GetOutputString(o), name) == 0) { *out = o; return true; }
    }
    return false;
}

static bool ParseModuleString(const char* name, EModule* out)
{
    for (int i = 1; i <= 3; ++i) {
        EModule m = (EModule)i;
        if (strcmp(GetModuleString(m), name) == 0) { *out = m; return true; }
    }
    return false;
}

// ---------------------------------------------------------------------------
// 模块切换 (GPIO 40 触发)
// ---------------------------------------------------------------------------
EModule CSilkLegGuitar::GetModule() { return s_module; }

bool CSilkLegGuitar::SwitchToModule(EModule m)
{
    s_module = m;
    CUserConfigure::SetString("module", GetModuleString(m));
    CUserConfigure::Commit();
#if defined(__has_include)
#  if __has_include("driver/gpio.h")
    // TODO: 原时序 (脉冲宽度/极性) RE 未完整反出 —— 占位:
    // gpio_set_level(GPIO_NUM_40, 1); esp_rom_delay_us(...); 0;
#  endif
#endif
    printf("SwitchToModule %s\n", GetModuleString(m));
    return true;
}

// ---------------------------------------------------------------------------
// 输出切换 (GPIO 40/2/32/14 序列)
// ---------------------------------------------------------------------------
EOutput CSilkLegGuitar::GetOutput() { return s_output; }

bool CSilkLegGuitar::SwitchOutput(EOutput o)
{
    s_output = o;
#if defined(__has_include)
#  if __has_include("driver/gpio.h")
    // TODO: GPIO 40/2/32/14 时序序列 (RE: 按档位组合拉脚, 细节未反出)
    // 每档位对应一组脚电平: none/line_out/speaker/wireless_microphone/
    // bluetooth_drive_speaker/bluetooth_midi_keyboard
#  endif
#endif
    printf("SwitchOutput %s\n", GetOutputString(o));
    // bluetooth_drive_speaker / bluetooth_midi_keyboard 还要求对应蓝牙链路
    // 生效 —— 由 JsInterface 各命令侧处理 (TODO 联动)
    return true;
}

// ---------------------------------------------------------------------------
// 电机
// ---------------------------------------------------------------------------
bool CSilkLegGuitar::MountMotor(int index, const char* dir, uint32_t ms)
{
    // TODO: 电机 GPIO 引脚号 RE 未反出; dir 仅接受 "Up"/"Down" (JS _8187)
    printf("MountMotor %d %s %ums\n", index, dir, (unsigned)ms);
    return true;
}

bool CSilkLegGuitar::SetAutoPitch(bool on)
{
    return on ? SwitchToModule(EModule::auto_pitch) : true;    // TODO: 关闭语义
}

bool CSilkLegGuitar::SetPickup(bool on)
{
    return on ? SwitchToModule(EModule::pickup) : true;
}

bool CSilkLegGuitar::SetMidiDevice(bool on)
{
    return on ? SwitchToModule(EModule::midi_device) : true;
}
