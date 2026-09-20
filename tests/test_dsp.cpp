// ============================================================================
// test_dsp.cpp —— DSP 功能还原宿主测试 (非 ESP-IDF 构建, 独立编译)
// ----------------------------------------------------------------------------
// 编译 (msys64 GCC, 中文标识符需 GCC>=10):
//   g++ -std=c++17 -Wall -Wextra -I firmware/main tests/test_dsp.cpp ^
//       firmware/main/FFT.cpp firmware/main/AutoPitch.cpp ^
//       firmware/main/PluckDetector.cpp -o build/test_dsp.exe
// 用例:
//   1. FFT: 1000Hz 正弦 512 点 → 峰 bin=32 (bin 宽 31.25Hz)
//   2. FFT: ForwardReal → InverseReal 往返误差 < 1e-3
//   3. PluckDetector: 0.3s 静音后 220Hz(+泛音) 拨弦 → 触发且音高≈220
//   4. AutoPitch: 构造 raw_short 歌曲文件 → 载入/解码/Add/读取输出
//      (包络逐点核对 + 置零段核对)
//   5. AutoPitch: 超长音符 (1200 > 自动乐器_音符数据最大长度) → printf
//      告警分支 + 截断 + 二次 Add 补齐
// ============================================================================

#include "../firmware/main/FFT.h"
#include "../firmware/main/AutoPitch.h"
#include "../firmware/main/PluckDetector.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int g_失败数 = 0;
static int g_通过数 = 0;

#define CHECK(条件, 消息)                                                      \
    do {                                                                       \
        if (条件) {                                                            \
            ++g_通过数;                                                        \
        } else {                                                               \
            ++g_失败数;                                                        \
            std::printf("  [FAIL] %s (line %d)\n", 消息, __LINE__);            \
        }                                                                      \
    } while (0)

static void 写BE32(std::vector<uint8_t>& 缓冲, size_t 偏移, uint32_t 值)
{
    缓冲[偏移 + 0] = (uint8_t)(值 >> 24);
    缓冲[偏移 + 1] = (uint8_t)(值 >> 16);
    缓冲[偏移 + 2] = (uint8_t)(值 >> 8);
    缓冲[偏移 + 3] = (uint8_t)(值);
}

static void 写LE16(std::vector<uint8_t>& 缓冲, size_t 偏移, uint16_t 值)
{
    缓冲[偏移 + 0] = (uint8_t)(值 & 0xFF);
    缓冲[偏移 + 1] = (uint8_t)(值 >> 8);
}

// ---------------------------------------------------------------------------
// 用例 1+2: FFT
// ---------------------------------------------------------------------------
static void 测FFT()
{
    std::printf("[TEST] FFT\n");
    constexpr int n = 512;
    std::vector<float> 信号(n);
    for (int i = 0; i < n; ++i)
        信号[i] = (float)std::sin(2.0 * M_PI * 1000.0 * (double)i / 16000.0);

    std::vector<float> 谱(n / 2);
    slg_fft::MagnitudeSpectrum(信号.data(), n, 谱.data());

    int 峰bin = 0;
    for (int k = 1; k < n / 2; ++k)
        if (谱[k] > 谱[峰bin])
            峰bin = k;
    CHECK(峰bin == 32, "1000Hz -> peak bin == 32");

    // 往返: 正弦 220.5Hz (非整 bin, 检验整体精度)
    for (int i = 0; i < n; ++i)
        信号[i] = (float)std::sin(2.0 * M_PI * 220.5 * (double)i / 16000.0);
    std::vector<float> re(n / 2 + 1), im(n / 2 + 1), 还原(n);
    CHECK(slg_fft::ForwardReal(信号.data(), n, re.data(), im.data()),
          "ForwardReal ok");
    CHECK(slg_fft::InverseReal(re.data(), im.data(), n, 还原.data()),
          "InverseReal ok");
    float 最大误差 = 0.0f;
    for (int i = 0; i < n; ++i) {
        const float 误差 = std::fabs(还原[i] - 信号[i]);
        if (误差 > 最大误差)
            最大误差 = 误差;
    }
    CHECK(最大误差 < 1e-3f, "roundtrip max error < 1e-3");

    // 非法参数: 非 2 幂
    std::vector<float> 小谱(8);
    slg_fft::MagnitudeSpectrum(信号.data(), 100, 小谱.data());   // 不崩溃即可
    ++g_通过数;   // 容错路径无断言即通过
}

// ---------------------------------------------------------------------------
// 用例 3: PluckDetector —— 0.3s 静音 + 220Hz 拨弦
// ---------------------------------------------------------------------------
static void 测拨弦检测()
{
    std::printf("[TEST] PluckDetector\n");
    CPluckDetector::初始化();

    constexpr int 采样率 = slg::kSampleRate;
    constexpr int 起始   = 采样率 * 3 / 10;        // 0.3s 静音
    constexpr int 时长   = 采样率 * 6 / 10;        // 总 0.6s
    std::vector<float> 全部(时长);
    for (int i = 0; i < 时长; ++i) {
        if (i < 起始) {
            全部[i] = 0.0f;   // 真静音
            continue;
        }
        const double t = (double)(i - 起始) / 采样率;
        const double 衰减 = std::exp(-3.0 * t);
        全部[i] = (float)(0.4 * 衰减 *
            (std::sin(2.0 * M_PI * 220.0 * t) +
             0.5 * std::sin(2.0 * M_PI * 440.0 * t) +
             0.25 * std::sin(2.0 * M_PI * 660.0 * t)));
    }

    constexpr int hop = 160;   // 10ms 帧
    bool  已触发 = false;
    float 音高 = 0.0f, 强度 = 0.0f;
    bool  方向 = false;
    int   帧号 = -1;
    for (int i = 0; i + hop <= 时长; i += hop) {
        CPluckDetector::喂样本(&全部[i], hop);
        bool 触发 = false;
        float f音高 = 0.0f, f强度 = 0.0f;
        bool f方向 = false;
        if (CPluckDetector::Go(触发, f音高, f强度, f方向) && 触发) {
            已触发 = true;
            音高 = f音高;
            强度 = f强度;
            方向 = f方向;
            帧号 = i / hop;
            break;
        }
    }
    CHECK(已触发, "pluck triggered");
    if (已触发) {
        std::printf("  触发帧=%d (%.0fms), 音高=%.2fHz, 强度=%.3f, 方向=%d\n",
                    帧号, 帧号 * 10.0f, 音高, 强度, (int)方向);
        CHECK(音高 >= 210.0f && 音高 <= 230.0f, "pitch ~= 220Hz (+-10)");
        CHECK(强度 > 0.0f && 强度 <= 1.0f, "strength in (0,1]");
        CHECK(帧号 >= 30, "no false trigger during silence");
    }
}

// ---------------------------------------------------------------------------
// 构造内存歌曲文件 (raw_short)
//   头 4096B: 魔数@0 / "1.0"@4 / 采样率@12 / 声道@16 / 格式串@20
//   目录 @0x1000: 24B 条目 {偏移,长度,采样率,格式,音符ID,保留} 全 BE
//   音频 @0x7000: s16LE PCM
// ---------------------------------------------------------------------------
static std::vector<uint8_t> 构造歌曲(int 样本数, double 频率, float 幅度)
{
    std::vector<uint8_t> 文件(0x7000 + (size_t)样本数 * 2, 0);
    写BE32(文件, 0, slg::kSongFileMagic);
    std::memcpy(&文件[4], "1.0", 3);
    写BE32(文件, 12, (uint32_t)slg::kSampleRate);
    写BE32(文件, 16, 1);
    std::memcpy(&文件[20], "raw_short", 9);

    const size_t 条目 = 0x1000;
    写BE32(文件, 条目 + 0, 0x7000);
    写BE32(文件, 条目 + 4, (uint32_t)样本数 * 2);
    写BE32(文件, 条目 + 8, (uint32_t)slg::kSampleRate);
    写BE32(文件, 条目 + 12, 0);
    写BE32(文件, 条目 + 16, 0);
    写BE32(文件, 条目 + 20, 0);

    for (int i = 0; i < 样本数; ++i) {
        const double t = (double)i / (double)slg::kSampleRate;
        const double 包络 = std::exp(-2.0 * t);
        const double s = 幅度 * 包络 * std::sin(2.0 * M_PI * 频率 * t);
        写LE16(文件, 0x7000 + (size_t)i * 2, (uint16_t)(int16_t)(s * 32767.0));
    }
    return 文件;
}

// 测试侧复算的包络 (与 AutoPitch.h 注释一致的 ◐ 公式)
static float 期望包络(int 位置, int 数据长度)
{
    const float 起音 = (位置 < 64) ? ((float)(位置 + 1) / 64.0f) : 1.0f;
    const int   剩余 = 数据长度 - 位置;
    const float 尾音 = (剩余 < 256) ? ((float)剩余 / 256.0f) : 1.0f;
    return 起音 * 尾音;
}

// ---------------------------------------------------------------------------
// 用例 4: AutoPitch 正常音符
// ---------------------------------------------------------------------------
static void 测自动乐器_正常()
{
    std::printf("[TEST] AutoPitch (raw_short, 200 样本)\n");
    CAutoPitch::初始化();

    auto 歌曲 = 构造歌曲(200, 440.0, 0.25f);
    CHECK(CAutoPitch::载入歌曲文件(歌曲.data(), (int)歌曲.size()),
          "载入歌曲文件");
    CHECK(CAutoPitch::音符数量() == 1, "音符数量 == 1");

    CHECK(!CAutoPitch::解码音符并放入缓存(-1), "越界索引拒绝");
    CHECK(CAutoPitch::解码音符并放入缓存(0), "解码音符并放入缓存(0)");

    S音符缓存* 音符 = CAutoPitch::取音符缓存(0);
    CHECK(音符 != nullptr && 音符->有效, "音符缓存有效");
    CHECK(音符->数据长度 == 200, "数据长度 == 200");

    CAutoPitch::Add(*音符, 1.0f);
    std::vector<float> 输出(1024);
    CHECK(CAutoPitch::读取输出(输出.data(), 1024) == 1024, "读取输出 1024");

    // 逐点核对: 输出[i] = 波形[i] × 音量 × 包络(i); [200..] 置零
    bool 逐点 = true, 置零段 = true;
    for (int i = 0; i < 200; ++i) {
        const float 期望 = 音符->波形[i] * 1.0f * 期望包络(i, 200);
        if (std::fabs(输出[i] - 期望) > 1e-4f)
            逐点 = false;
    }
    for (int i = 200; i < 1024; ++i)
        if (输出[i] != 0.0f)
            置零段 = false;
    CHECK(逐点, "envelope render matches");
    CHECK(置零段, "zero-fill tail");

    CAutoPitch::释放();
}

// ---------------------------------------------------------------------------
// 用例 5: AutoPitch 超长音符 (printf 告警分支 + 截断续播)
// ---------------------------------------------------------------------------
static void 测自动乐器_超长()
{
    std::printf("[TEST] AutoPitch (1200 样本 > 自动乐器_音符数据最大长度)\n");
    CAutoPitch::初始化();

    auto 歌曲 = 构造歌曲(1200, 330.0, 0.3f);
    CHECK(CAutoPitch::载入歌曲文件(歌曲.data(), (int)歌曲.size()),
          "载入歌曲文件");
    CHECK(CAutoPitch::解码音符并放入缓存(0), "解码音符并放入缓存(0)");

    S音符缓存* 音符 = CAutoPitch::取音符缓存(0);
    CHECK(音符 != nullptr && 音符->数据长度 == 1200, "数据长度 == 1200");

    std::printf("  (下方一行应为固件实锤 printf 告警)\n");
    CAutoPitch::Add(*音符, 0.5f);   // 剩余 1200 > 1024 → printf + 截断
    CHECK(音符->播放位置 == 1024, "Add#1 播放位置 == 1024");

    std::vector<float> 输出(1024);
    CAutoPitch::读取输出(输出.data(), 1024);
    bool 第一块有声 = false;
    for (int i = 0; i < 1024; ++i)
        if (std::fabs(输出[i]) > 0.01f) { 第一块有声 = true; break; }
    CHECK(第一块有声, "first block audible");

    CAutoPitch::Add(*音符, 0.5f);   // 剩余 176 + 置零 848
    CHECK(音符->播放位置 == 1200, "Add#2 播放位置 == 1200 (完整)");
    CAutoPitch::读取输出(输出.data(), 1024);
    bool 尾段逐点 = true, 尾段置零 = true;
    for (int i = 0; i < 176; ++i) {
        const float 期望 = 音符->波形[1024 + i] * 0.5f * 期望包络(1024 + i, 1200);
        if (std::fabs(输出[i] - 期望) > 1e-4f)
            尾段逐点 = false;
    }
    for (int i = 176; i < 1024; ++i)
        if (输出[i] != 0.0f)
            尾段置零 = false;
    CHECK(尾段逐点, "second block matches remaining samples");
    CHECK(尾段置零, "second block zero-fill");

    CAutoPitch::释放();
}

int main()
{
    std::printf("=== silk-leg-guitar DSP host tests ===\n");
    测FFT();
    测拨弦检测();
    测自动乐器_正常();
    测自动乐器_超长();
    std::printf("=== PASS %d, FAIL %d ===\n", g_通过数, g_失败数);
    return g_失败数 == 0 ? 0 : 1;
}
