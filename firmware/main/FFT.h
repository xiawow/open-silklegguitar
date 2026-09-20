// ============================================================================
// FFT.h —— 频谱分析 (radix-2 实 FFT)
// ----------------------------------------------------------------------------
// 还原依据: 固件 FFT 库断言串 "FFT failed in IFFTReal" / "Bit rev failed in
// IFFTReal" (E (%lu) %s: 格式) —— 原库含 IFFTReal (实数逆变换), 推断配套有
// 实数正变换。原实现为 Xtensa 优化库, 此处按同接口语义用 radix-2 Cooley-Tukey
// 重写 (C++ 可移植版, 数值行为等价)。
// 用途: CPluckDetector 谱峰检测 (512 点窗, 16kHz, bin 宽 31.25Hz)。
// ============================================================================
#pragma once

#include <cstdint>

namespace slg_fft {

// 支持的最大点数 (radix-2: n 必须为 2 的幂且 ≤ 此值)
constexpr int kMaxFFTSize = 4096;

// 实数 n 点 FFT → 半谱 (含 Nyquist)。
//   input : 时域样本 [0..n-1]
//   reOut / imOut : 半谱 reOut[0..n/2], imOut[0..n/2] (调用方保证 n/2+1 容量)
// 返回 false: n 非 2 的幂 / 超上限 / 空指针。
bool ForwardReal(const float* input, int n, float* reOut, float* imOut);

// 半谱 → 实数时域信号 (对应固件 IFFTReal)。
//   reIn / imIn : 半谱 [0..n/2] (Nyquist bin 的虚部按 0 处理)
//   output      : 时域 [0..n-1]
// 返回 false: 参数非法。利用共轭对称补全整谱后做逆变换并 /n。
bool InverseReal(const float* reIn, const float* imIn, int n, float* output);

// 幅度谱便捷封装: n 点实信号 → magOut[0..n/2-1] (不含 Nyquist bin)。
void MagnitudeSpectrum(const float* input, int n, float* magOut);

} // namespace slg_fft
