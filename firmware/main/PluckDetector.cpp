// ============================================================================
// PluckDetector.cpp —— 拨弦检测实现
// ----------------------------------------------------------------------------
// 数据流: 喂样本 → 512 样本环 → Hann 窗 → 512 点实 FFT (slg_fft) → 幅度谱
//         → ±12 邻域峰判定 (≥5 更小) → 峰数滑动平均 → 65/33 起振判定 → 触发
// 峰判定 / 起振魔数 / 数组容量均为反汇编实锤; 窗长 512、历史窗 50 帧、
// 不应期 20 帧、噪声门 1e-3 为 ◐ 推断参数 (实机可调)。
// ============================================================================

#include "PluckDetector.h"

#include "FFT.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---------------------------------------------------------------------------
// 静态成员定义
// ---------------------------------------------------------------------------
float     CPluckDetector::m样本环[CPluckDetector::k窗长];
float     CPluckDetector::m窗样本[CPluckDetector::k窗长];
float     CPluckDetector::m谱[CPluckDetector::k谱点数];
int       CPluckDetector::m写入游标   = 0;
int       CPluckDetector::m累计新样本 = 0;
bool      CPluckDetector::m首帧已完成 = false;
int       CPluckDetector::m峰数历史[CPluckDetector::k历史帧数] = {0};
int       CPluckDetector::m历史写入   = 0;
int       CPluckDetector::m历史计数   = 0;
int       CPluckDetector::m自上次触发帧 = 1000;
int       CPluckDetector::m候选帧数   = 0;
S山峰数组 CPluckDetector::m当前峰数组;

namespace {

// Hann 窗 (周期形式, 分母 n —— 谱分析标准做法)
inline float 汉宁窗(int i, int n)
{
    return 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * (float)i / (float)n));
}

} // namespace

// ---------------------------------------------------------------------------
void CPluckDetector::初始化()
{
    for (int i = 0; i < k窗长; ++i) {
        m样本环[i] = 0.0f;
        m窗样本[i] = 0.0f;
    }
    for (int i = 0; i < k谱点数; ++i)
        m谱[i] = 0.0f;
    m写入游标     = 0;
    m累计新样本   = 0;
    m首帧已完成   = false;
    for (int i = 0; i < k历史帧数; ++i)
        m峰数历史[i] = 0;
    m历史写入     = 0;
    m历史计数     = 0;
    m自上次触发帧 = 1000;
    m候选帧数     = 0;
    m当前峰数组.清零();
}

void CPluckDetector::喂样本(const float* 样本, int n)
{
    if (样本 == nullptr)
        return;
    for (int i = 0; i < n; ++i) {
        m样本环[m写入游标] = 样本[i];
        m写入游标 = (m写入游标 + 1) % k窗长;
    }
    m累计新样本 += n;   // 记"自上次取帧以来新增数", Go 按 hop 消费
}

bool CPluckDetector::Go(bool& 触发, float& 音高, float& 强度, bool& 方向)
{
    // 反汇编实锤: 入口先清零 4 个出参
    触发 = false;
    音高 = 0.0f;
    强度 = 0.0f;
    方向 = false;

    // 帧调度: 首帧需满窗, 之后每凑满一个 hop 处理一帧
    const bool 够首帧 = (m累计新样本 >= k窗长);
    const bool 够后续 = (m首帧已完成 && m累计新样本 >= k跳长);
    if (!够首帧 && !够后续)
        return false;
    m累计新样本 -= k跳长;
    m首帧已完成 = true;

    // ---- 取最新 512 样本 + 加窗 ----
    for (int i = 0; i < k窗长; ++i) {
        const int 源 = (m写入游标 - k窗长 + i + 2 * k窗长) % k窗长;
        m窗样本[i] = m样本环[源] * 汉宁窗(i, k窗长);
    }

    // ---- FFT → 幅度谱 ----
    slg_fft::MagnitudeSpectrum(m窗样本, k窗长, m谱);

    // ---- 谱峰判定 (实锤: 中心 bin 与 ±12 邻域逐个比较, ≥5 个更小) ----
    m当前峰数组.清零();
    float 谱和 = 0.0f;
    for (int b = 0; b < k谱点数; ++b)
        谱和 += m谱[b];

    for (int b = 0; b < k谱点数; ++b) {
        const float 中心 = m谱[b];
        if (中心 <= k噪声门)
            continue;
        int 更小数 = 0;
        for (int d = -k峰邻域; d <= k峰邻域; ++d) {
            if (d == 0)
                continue;
            const int j = b + d;
            if (j < 0 || j >= k谱点数)
                continue;   // 低频段 (如 220Hz→bin7) 邻域截边, 允许不足 24 个
            if (m谱[j] < 中心)
                ++更小数;
        }
        if (更小数 < k峰最少邻居)
            continue;

        S山峰 峰;
        峰.频率 = (float)b * (float)slg::kSampleRate / (float)k窗长;
        峰.幅度 = 中心;
        峰.频点 = b;
        if (!m当前峰数组.加入(峰)) {
            // 容量 16 满: 替换最弱峰 (原版断言 16||8, 溢出行为未见, ◐)
            int 最弱 = 0;
            for (int j = 1; j < S山峰数组::容量; ++j)
                if (m当前峰数组.条目[j].幅度 < m当前峰数组.条目[最弱].幅度)
                    最弱 = j;
            if (峰.幅度 > m当前峰数组.条目[最弱].幅度)
                m当前峰数组.条目[最弱] = 峰;
        }
    }

    const int 峰数 = m当前峰数组.数量;

    // ---- 峰数滑动平均 (不含本帧 → 起振 = 突增) ----
    float 平均 = 0.0f;
    if (m历史计数 > 0) {
        for (int i = 0; i < m历史计数; ++i)
            平均 += (float)m峰数历史[i];
        平均 /= (float)m历史计数;
    }
    m峰数历史[m历史写入] = 峰数;
    m历史写入 = (m历史写入 + 1) % k历史帧数;
    if (m历史计数 < k历史帧数)
        ++m历史计数;
    ++m自上次触发帧;

    // ---- 起振判定 (实锤: 峰数 ≥ 65×滑动平均/33; 不满足则清状态) ----
    const float 起振门限 = k起振分子 * 平均 / k起振分母;
    const bool  起振     = (峰数 >= 2) && ((float)峰数 > 起振门限);
    if (!起振) {
        m候选帧数 = 0;   // 清状态
        return true;
    }
    ++m候选帧数;
    if (m候选帧数 < k起振确认帧)
        return true;   // 起振确认: 等信号前沿填满窗 (否则瞬态峰偏移, 音高失真)
    if (m自上次触发帧 <= k不应期帧数)
        return true;

    // ---- 触发: 取最强峰为音高 ----
    int 最强 = 0;
    for (int j = 1; j < 峰数; ++j)
        if (m当前峰数组.条目[j].幅度 > m当前峰数组.条目[最强].幅度)
            最强 = j;

    触发 = true;
    音高 = m当前峰数组.条目[最强].频率;
    强度 = (谱和 > 1e-9f) ? (m当前峰数组.条目[最强].幅度 / 谱和) : 0.0f;

    // 方向 (◐ 推测: 拨弦方向) —— 用帧末 64 样本一阶差分均值符号
    {
        float 斜率和 = 0.0f;
        for (int i = k窗长 - 63; i < k窗长; ++i)
            斜率和 += (m窗样本[i] - m窗样本[i - 1]);
        方向 = (斜率和 > 0.0f);
    }

    m自上次触发帧 = 0;
    m候选帧数     = 0;
    return true;
}
