// ============================================================================
// AutoPitch.h —— 自动乐器音符播放引擎 (CAutoPitch)
// ----------------------------------------------------------------------------
// ★ 架构修正 (RE 实锤): 本模块不是"输入音高检测", 而是自动乐器 (auto_pitch)
//   音符播放引擎。数据流: 解码音符并放入缓存 → PCM → 音量包络 → PWM
//   (m_to_pwm_index)。
//
// 标识符全部为固件实锤 (断言串/printf 格式串):
//   * CAutoPitch::初始化()      —— sub_400DEA9C: 断言 5 成员==NULL (行
//     54/58/62/66/72) → 分配 3 块缓存 + 音符缓存数组(0xFF 条) →
//     sub_400E499C() 创建 mp3 解码器。
//   * 解码音符并放入缓存(int)   —— sub_400DEDCC (行 0x457)。
//   * Add(S音符缓存&, float)    —— sub_400DF2B0 (行 0x2c2); printf 格式串:
//     "CAutoPitch::Add m_音量保持缓存_index=%lld, m_to_pwm_index=%lld
//      剩余数据长度=%d 置零长度=%d > 自动乐器_音符数据最大长度=%d"
//   * 成员: m_音量保持缓存 / m_音量衰减缓存 / m目录 / m音符缓存数组 /
//     m_mp3_decoder / m当前音符索引 / m音符数量 / m_to_pwm_index /
//     m_音量保持缓存_index / S音符缓存
// 歌曲文件格式 (校验串在 AutoPitch.cpp 字符串区): 魔数 0x2B047D6A /
// 版本 "1.0" / 采样率 / 声道==1 / "raw_short"|"mp3" / 目录扇区上限;
// 头 4096B, 目录条目 @0x1000 每条 24B, 音频区 @0x7000。
// ◐ 推断部分 (逐处标注): 第三块缓存名/包络曲线形状/mp3 解码器本体
// (sub_400E499C 未还原, 桩)/目录条目 24B 字段细分。
// ============================================================================
#pragma once

#include "Common.h"

#include <cstdint>

// printf 格式串实锤的常量名 (值 1024: 与缓存块长度一致 ◐)
constexpr int 自动乐器_音符数据最大长度 = 1024;

// 音符缓存条目 (固件符号实锤: S音符缓存)
struct S音符缓存 {
    float* 波形     = nullptr;   // 解码后 PCM (float 单声道, -1..1)
    int    数据长度 = 0;        // 样本数
    int    播放位置 = 0;        // 当前播放游标 (样本)
    int    音符索引 = -1;       // 歌曲目录条目号
    bool   有效     = false;
};

class CAutoPitch {
public:
    // 固件实锤: 断言 5 成员==NULL → 分配缓存 → 创建 mp3 解码器
    static void 初始化();
    // 骨架补充: 释放全部资源 (回到可重新 初始化() 的状态)
    static void 释放();

    // 载入歌曲文件 (内存镜像; BE 头解析 + 目录装载)
    // 校验: 魔数 0x2B047D6A / 版本 "1.0" / 采样率 16k / 声道==1 /
    //       格式串 "raw_short" 或 "mp3"
    static bool 载入歌曲文件(const void* 数据, int 长度);

    // 固件实锤 sub_400DEDCC: 按目录条目解码音频 → 放入 m音符缓存数组[索引]
    // raw_short: s16LE → float 直转; mp3: 解码器未还原 → 失败并打印 TODO
    static bool 解码音符并放入缓存(int 音符索引);

    // 固件实锤 sub_400DF2B0: 把音符剩余 PCM 按音量包络排入输出环
    // (m_音量保持缓存), 尾部置零; 剩余超长走 printf 实锤告警分支并截断
    static void Add(S音符缓存& 音符, float 音量);

    // 消费输出环 (PWM 数据流落点; 推进 m_to_pwm_index)。返回实际读取样本数
    static int  读取输出(float* 输出, int n);

    // 调试/测试访问
    static int          音符数量()     { return m音符数量; }
    static S音符缓存*   取音符缓存(int 音符索引)
    {
        if (m音符缓存数组 == nullptr || 音符索引 < 0 || 音符索引 >= 0xFF)
            return nullptr;
        return &m音符缓存数组[音符索引];
    }

private:
    // 音量包络 (◐): 起音查 m_包络缓存 表 (64 样本线性爬升) × 尾音线性淡出
    static float 包络值(int 位置, int 数据长度);

    // ---- 成员 (名字全部固件实锤; m_包络缓存 为第三块缓存 ◐ 命名) ----
    static float*      m_音量保持缓存;    // 播放输出环 (printf 实锤)
    static float*      m_音量衰减缓存;    // 尾音衰减环 (printf 实锤)
    static float*      m_包络缓存;        // 第三块缓存 ◐ (起音包络表)
    static S音符缓存*  m音符缓存数组;     // 0xFF 条 (实锤)
    static void*       m_mp3_decoder;     // sub_400E499C 产物 (未还原 → 桩)
    static const uint8_t* m目录;         // 歌曲目录区基址 (文件内 @0x1000)
    static const uint8_t* m歌曲数据;     // ◐ 载入的歌曲文件副本基址
    static int         m歌曲长度;
    static int         m格式;            // 0=raw_short, 1=mp3

    static int64_t     m_音量保持缓存_index;  // printf %lld 实锤
    static int64_t     m_to_pwm_index;        // printf %lld 实锤
    static int         m当前音符索引;         // 实锤成员名
    static int         m音符数量;             // 实锤成员名

    // 歌曲文件布局常量 (校验串 + JS _1210._8341 实锤; 条目字段细分 ◐)
    static constexpr uint32_t k歌曲头长度     = 4096;
    static constexpr uint32_t k目录偏移       = 0x1000;
    static constexpr uint32_t k音频区偏移     = 0x7000;
    static constexpr uint32_t k目录条目长度   = 24;
    static constexpr uint32_t k目录条目上限   = (k音频区偏移 - k目录偏移) / k目录条目长度; // 目录扇区上限
};
