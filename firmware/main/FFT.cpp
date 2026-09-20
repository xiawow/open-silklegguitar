// ============================================================================
// FFT.cpp —— 朴素 DFT 占位实现 (TODO: 换 radix-2 / esp-dsp)
// ============================================================================

#include "FFT.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace slg_fft {

void MagnitudeSpectrum(const float* input, int n, float* magOut)
{
    if (input == nullptr || magOut == nullptr || n <= 0)
        return;
    const int half = n / 2;
    for (int k = 0; k < half; ++k) {
        double re = 0.0, im = 0.0;
        const double ang = -2.0 * M_PI * (double)k / (double)n;
        for (int t = 0; t < n; ++t) {
            const double a = ang * t;
            re += (double)input[t] * cos(a);
            im += (double)input[t] * sin(a);
        }
        magOut[k] = (float)sqrt(re * re + im * im);
    }
}

} // namespace slg_fft
