// ============================================================================
// RecordTemporarily.h —— 临时录音 (RecordTemporarily / GetHistoryRecord 命令)
// ----------------------------------------------------------------------------
// 还原来源: _27.js _8164/_8165 (命令) + _8142 (响应消费, 置信度 ★★★★★)
//   * 命令负载: string "input" / "output" (输入=麦克风 / 输出=混音)
//   * 响应负载: i32 BE 采样率 + PCM (LE u16, 每样本 = s16 + 0x8000 偏置)
//     —— 音频批量数据小端 memcpy, 协议整数大端, 两者并存 (JS new
//     Uint16Array() 原生端序解释实证)
//   * input→RecordTemporarilyMonoData / output→StereoData (通道 1/2, ◐)
// ============================================================================
#pragma once

#include "Common.h"

class CRecordTemporarily {
public:
    enum class Source { input, output };     // 对应命令负载字符串

    static void Init();
    static bool Start(Source src);
    static void Stop();
    static bool IsRecording()   { return s_recording; }
    static Source GetSource()   { return s_source; }

    // 音频链路喂样本 (s16 原始); 内部转 u16 偏置 (s16 + 0x8000) 存储
    static void Feed(const int16_t* samples, int count);

    // 已录数据 (u16 偏置格式, 可直接 memcpy 进响应包)
    static const uint16_t* Data()        { return s_buffer; }
    static int  SampleCount()            { return s_count; }
    static int  Capacity()               { return kCapacity; }

    // TODO: 实际容量 RE 未反出 (骨架按 10 秒 @16kHz)
    static constexpr int kCapacity = 16000 * 10;

private:
    static bool      s_recording;
    static Source    s_source;
    static uint16_t  s_buffer[kCapacity];
    static int       s_count;
};
