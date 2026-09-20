// ============================================================================
// PluckDetector.h —— 拨弦检测 (频域谱峰 + 起振判定)
// ----------------------------------------------------------------------------
// 还原依据 (反汇编级实锤, 见逆向报告):
//   * Go 本体位于未分析空洞 0x400E050C 附近 (entry sp,0x150 大栈帧,
//     入口先清零 4 个出参 → 原签名 Go(bool&, float&, float&, bool&))。
//   * 谱峰判定: 中心 bin 与 ±12 邻域逐个 (olt.s) 比较, ≥5 个更小才算峰
//     (blti a4,5)。
//   * 起振判定: 当前峰数 ≥ ~2×滑动平均 (魔数除法 65*avg/33 ≈ 1.97);
//     不满足则清状态。
//   * 山峰数组容量 16 (断言 "output_length == 16 || output_length == 8")。
//   * 谱历史环: 全局基址 dword_400D1080+0x9C, 每帧一条记录 (160B)。
//   * 配套 FFT 库断言串 "FFT failed in IFFTReal" (见 FFT.h)。
// 结构名 S山峰/S山峰数组 为固件符号实锤; Go 出参形参名按语义还原 (◐)。
// ============================================================================
#pragma once

#include "Common.h"

// 谱峰记录 (固件符号 S山峰数组::S山峰 实锤; 字段名按语义还原 ◐)
struct S山峰 {
    float 频率;   // Hz = 频点 × 采样率 / FFT 窗长
    float 幅度;   // 谱幅 (线性)
    int   频点;   // FFT bin 序号
};

// 山峰数组 (容量实锤: 断言 16 || 8; 骨架按 16 实现)
struct S山峰数组 {
    static constexpr int 容量 = 16;

    S山峰 条目[容量];
    int   数量 = 0;

    void 清零() { 数量 = 0; }
    bool 加入(const S山峰& 峰)
    {
        if (数量 >= 容量)
            return false;
        条目[数量] = 峰;
        ++数量;
        return true;
    }
};

class CPluckDetector {
public:
    // 清谱历史/状态 (原版: dword_400D1080 全局区初始化)
    static void 初始化();

    // 喂入新样本 (16kHz 单声道 float; 内部 512 样本环形窗)
    static void 喂样本(const float* 样本, int n);

    // 原签名 (反汇编实锤: 4 出参, 入口清零)。
    // 每调用消费一帧 (hop 160 样本 = 10ms @16k); 有完整帧时返回 true,
    // 触发拨弦时 触发=true 且输出 音高(Hz)/强度(0..1)/方向(◐: 末段斜率符号)。
    static bool Go(bool& 触发, float& 音高, float& 强度, bool& 方向);

private:
    // ---- 引擎常量 ----
    static constexpr int   k窗长       = 512;  // ◐ FFT 窗长 (16k → bin 31.25Hz)
    static constexpr int   k跳长       = 160;  // 谱历史每帧一条 ↔ hop 160 样本 (10ms)
    static constexpr int   k谱点数     = k窗长 / 2;
    static constexpr int   k峰邻域     = 12;   // 实锤: ±12 邻域
    static constexpr int   k峰最少邻居 = 5;    // 实锤: blti a4,5
    static constexpr int   k历史帧数   = 50;   // ◐ 滑动平均窗 (0.5s)
    static constexpr int   k不应期帧数 = 20;   // ◐ 触发后 200ms 静默
    static constexpr int   k起振确认帧 = 3;   // ◐ 连续 3 帧起振才触发 (~30ms,
                                              //    等信号前沿填满窗 → 音高稳定)
    static constexpr float k噪声门     = 1e-3f;// ◐ 峰幅度绝对门限
    static constexpr float k起振分子   = 65.0f;// 实锤魔数 65/33 ≈ 1.9697
    static constexpr float k起振分母   = 33.0f;

    // ---- 状态 (原版为 dword_400D1080 全局区; 骨架用静态成员) ----
    static float        m样本环[k窗长];      // 输入环形窗
    static float        m窗样本[k窗长];      // 加窗后的帧
    static float        m谱[k谱点数];        // 幅度谱
    static int          m写入游标;
    static int          m累计新样本;
    static bool         m首帧已完成;
    static int          m峰数历史[k历史帧数];
    static int          m历史写入;
    static int          m历史计数;
    static int          m自上次触发帧;
    static int          m候选帧数;
    static S山峰数组    m当前峰数组;
};
