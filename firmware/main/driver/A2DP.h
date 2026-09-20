// ============================================================================
// A2DP.h —— A2DP Source (bluetooth_drive_speaker 输出路径)
// ----------------------------------------------------------------------------
// 前端 SetOutput("bluetooth_drive_speaker") 后音频经 A2DP 发往已连音箱。
// 源: 固件字符串/符号 + 前端 GetBluetoothDriveSpeakState 状态机。骨架桩。
// ============================================================================
#pragma once

#include "Common.h"

class CA2dpSource {
public:
    static void Init();
    static bool Start();       // 进入蓝牙音箱输出模式
    static void Stop();
    static bool WritePcm(const int16_t* interleavedStereo, int sampleCount); // S16 立体声
    static bool IsActive() { return s_active; }

private:
    static bool s_active;
};
