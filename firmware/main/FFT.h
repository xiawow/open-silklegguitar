// ============================================================================
// FFT.h —— 频谱分析 (AutoPitch 音高检测前置)
// ----------------------------------------------------------------------------
// 桩说明: 原实现被 NDEBUG 裁剪不可恢复 (逆向报告边界)。骨架用朴素 DFT
// (O(n²)) 占位, 仅保证接口与数据流形状; 实机建议换 esp-dsp 或手写 radix-2。
// ============================================================================
#pragma once

#include <cstdint>

namespace slg_fft {

// 实数输入 n 点 → n/2 幅度谱 (magOut[0..n/2-1])。
// 采样率 16kHz (slg::kSampleRate); AutoPitch 在频谱上找基频。
void MagnitudeSpectrum(const float* input, int n, float* magOut);

} // namespace slg_fft
