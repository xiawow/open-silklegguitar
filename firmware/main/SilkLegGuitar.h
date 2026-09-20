// ============================================================================
// SilkLegGuitar.h —— 整机业务层 (模块切换 / 输出切换 / 电机)
// ----------------------------------------------------------------------------
// 还原来源: 固件 SwitchToModule / SwitchOutput / MountMotor 反汇编 + 前端
//           _613.js 业务动作对照
//   * SwitchToModule: GPIO 40 触发 (RE: 单脉冲选择模块)
//   * SwitchOutput:   GPIO 40/2/32/14 时序序列 (RE: 5 输出档位)
//   * MountMotor:     电机 1/2 × Up/Down × 毫秒 (GPIO 引脚号 TODO)
// ============================================================================
#pragma once

#include "Common.h"

class CSilkLegGuitar {
public:
    static void Init();                            // GPIO 配置 + 从配置恢复状态

    static EModule GetModule();
    static bool    SwitchToModule(EModule m);      // GPIO 40 触发

    static EOutput GetOutput();
    static bool    SwitchOutput(EOutput o);        // GPIO 40/2/32/14 序列

    static bool    MountMotor(int index, const char* dir, uint32_t ms);

    // SetAutoPitch / SetPickup / SetMidiDevice 命令落点 (负载格式未反出,
    // 骨架按布尔开关处理 → 映射到 SwitchToModule)
    static bool SetAutoPitch(bool on);
    static bool SetPickup(bool on);
    static bool SetMidiDevice(bool on);
};

// GetOutputString / GetModuleString (Common.h 声明) —— 返回前端使用的确切
// 字符串 (固件字符串表 @0x4A08-0x4B2C), 实现见 SilkLegGuitar.cpp
