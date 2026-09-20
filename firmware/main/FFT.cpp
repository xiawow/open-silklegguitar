// ============================================================================
// FFT.cpp —— radix-2 Cooley-Tukey 实 FFT / IFFTReal
// ----------------------------------------------------------------------------
// 迭代式位反转 + 蝶形; twiddle 因子逐级递推 (double 递推, 长度 ≤4096 时误差
// 远低于 float 精度)。原固件为 Xtensa 汇编优化库 (不可逐指令还原), 本实现
// 保证同接口同数值语义。
// ============================================================================

#include "FFT.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace slg_fft {

namespace {

// n 是否为 2 的幂 (且非 0)
bool 是2的幂(int n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

// 位反转重排 (对应固件库 "Bit rev failed" 检查的那一步)
void 位反转重排(float* re, float* im, int n)
{
    int j = 0;
    for (int i = 1; i < n; ++i) {
        int bit = n >> 1;
        for (; (j & bit) != 0; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) {
            float tr = re[i]; re[i] = re[j]; re[j] = tr;
            float ti = im[i]; im[i] = im[j]; im[j] = ti;
        }
    }
}

// 蝶形运算。符号 = -1 正变换 (e^-iω), +1 逆变换 (e^+iω)。
void 蝶形(float* re, float* im, int n, int 符号)
{
    for (int len = 2; len <= n; len <<= 1) {
        const int 半 = len >> 1;
        const double ang = (double)符号 * 2.0 * M_PI / (double)len;
        const double wmr = std::cos(ang);
        const double wmi = std::sin(ang);
        for (int i = 0; i < n; i += len) {
            double wr = 1.0;
            double wi = 0.0;
            for (int j = 0; j < 半; ++j) {
                const int a = i + j;
                const int b = a + 半;
                const double br = (double)re[b];
                const double bi = (double)im[b];
                // (br,bi) × (wr,wi)
                const float tr = (float)(br * wr - bi * wi);
                const float ti = (float)(br * wi + bi * wr);
                re[b] = re[a] - tr;
                im[b] = im[a] - ti;
                re[a] += tr;
                im[a] += ti;
                // twiddle 递推 (double 累积, 精度足够)
                const double nwr = wr * wmr - wi * wmi;
                const double nwi = wr * wmi + wi * wmr;
                wr = nwr;
                wi = nwi;
            }
        }
    }
}

} // namespace

bool ForwardReal(const float* input, int n, float* reOut, float* imOut)
{
    if (input == nullptr || reOut == nullptr || imOut == nullptr)
        return false;
    if (!是2的幂(n) || n > kMaxFFTSize)
        return false;

    // 工作缓冲: 单线程音频引擎 (固件本身即全局谱历史区同款用法), 静态复用
    static float re[kMaxFFTSize];
    static float im[kMaxFFTSize];

    for (int i = 0; i < n; ++i) {
        re[i] = input[i];
        im[i] = 0.0f;
    }
    位反转重排(re, im, n);
    蝶形(re, im, n, -1);

    const int 半 = n / 2;
    for (int k = 0; k <= 半; ++k) {   // 含 Nyquist bin
        reOut[k] = re[k];
        imOut[k] = im[k];
    }
    return true;
}

bool InverseReal(const float* reIn, const float* imIn, int n, float* output)
{
    if (reIn == nullptr || imIn == nullptr || output == nullptr)
        return false;
    if (!是2的幂(n) || n > kMaxFFTSize)
        return false;

    static float re[kMaxFFTSize];
    static float im[kMaxFFTSize];

    // 半谱 → 共轭对称全谱 (Nyquist bin 按纯实数处理)
    const int 半 = n / 2;
    for (int k = 0; k < 半; ++k) {
        re[k] = reIn[k];
        im[k] = imIn[k];
    }
    re[半] = reIn[半];
    im[半] = 0.0f;
    for (int k = 半 + 1; k < n; ++k) {
        re[k] = re[n - k];
        im[k] = -im[n - k];
    }

    位反转重排(re, im, n);
    蝶形(re, im, n, +1);

    const float 归一 = 1.0f / (float)n;
    for (int i = 0; i < n; ++i)
        output[i] = re[i] * 归一;   // 实信号: 虚部残差 ≈ 0, 直接取实部
    return true;
}

void MagnitudeSpectrum(const float* input, int n, float* magOut)
{
    if (input == nullptr || magOut == nullptr)
        return;
    if (!是2的幂(n) || n > kMaxFFTSize)
        return;

    static float re[kMaxFFTSize / 2 + 1];
    static float im[kMaxFFTSize / 2 + 1];

    if (!ForwardReal(input, n, re, im))
        return;
    const int 半 = n / 2;
    for (int k = 0; k < 半; ++k)
        magOut[k] = std::sqrt(re[k] * re[k] + im[k] * im[k]);
}

} // namespace slg_fft
