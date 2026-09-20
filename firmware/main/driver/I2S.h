// ============================================================================
// I2S.h —— 音频输入/输出 (16kHz)
// ----------------------------------------------------------------------------
// 输入 = 麦克风 (ADC 路径: Adc1Dma/AdcPoll 为头文件式组件, 见 main/CMakeLists
// 注; 电压档位枚举 EADC1VoltageRange 值 RE 未反出 —— TODO)
// 输出 = line_out / speaker (ESource 选单/双声道)
// ============================================================================
#pragma once

#include "Common.h"

class CI2sDriver {
public:
    static void Init(int sampleRate, ESource channels);

    // 麦克风输入 (mono s16; 录音/AutoPitch/拾音器数据源)
    static bool ReadMono(int16_t* out, int samples);

    // 扬声器输出 (line_out / speaker 档位)
    static bool WriteStereo(const int16_t* l, const int16_t* r, int samples);

    static int SampleRate() { return s_rate; }

private:
    static int  s_rate;
    static ESource s_channels;
};
