// ============================================================================
// AutoPitch.cpp —— 自动乐器音符播放引擎实现
// ----------------------------------------------------------------------------
// 数据流 (RE 实锤): 解码音符并放入缓存 → PCM → 音量包络 → PWM (m_to_pwm_index)
// 包络形状/缓存块长/mp3 解码器为 ◐ 推断, 其余标识符与分支均为固件实锤。
// ============================================================================

#include "AutoPitch.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// ---------------------------------------------------------------------------
// 静态成员定义
// ---------------------------------------------------------------------------
float*             CAutoPitch::m_音量保持缓存    = nullptr;
float*             CAutoPitch::m_音量衰减缓存    = nullptr;
float*             CAutoPitch::m_包络缓存        = nullptr;
S音符缓存*         CAutoPitch::m音符缓存数组     = nullptr;
void*              CAutoPitch::m_mp3_decoder     = nullptr;
const uint8_t*     CAutoPitch::m目录             = nullptr;
const uint8_t*     CAutoPitch::m歌曲数据         = nullptr;
int                CAutoPitch::m歌曲长度         = 0;
int                CAutoPitch::m格式             = 0;
int64_t            CAutoPitch::m_音量保持缓存_index = 0;
int64_t            CAutoPitch::m_to_pwm_index       = 0;
int                CAutoPitch::m当前音符索引     = -1;
int                CAutoPitch::m音符数量         = 0;

namespace {

// 大端读取 (歌曲文件为 BE 格式)
inline uint32_t 读BE32(const uint8_t* p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}

// mp3 解码器 (sub_400E499C 未还原 → 桩句柄; TODO 实机接 helix mp3 等)
void* 创建mp3解码器桩()
{
    return std::malloc(4);   // 非空句柄占位
}

} // namespace

// ---------------------------------------------------------------------------
void CAutoPitch::初始化()
{
    // 固件实锤: 5 成员 == NULL 断言 (原文件行 54/58/62/66/72)
    SLG_ASSERT(m_音量保持缓存 == nullptr);
    SLG_ASSERT(m_音量衰减缓存 == nullptr);
    SLG_ASSERT(m_包络缓存     == nullptr);
    SLG_ASSERT(m音符缓存数组  == nullptr);
    SLG_ASSERT(m_mp3_decoder  == nullptr);

    // 3 块缓存 (RE: 3×1024B; 此处按 自动乐器_音符数据最大长度×float 实现 ◐)
    m_音量保持缓存 = new float[自动乐器_音符数据最大长度]();
    m_音量衰减缓存 = new float[自动乐器_音符数据最大长度]();
    m_包络缓存     = new float[自动乐器_音符数据最大长度]();
    for (int i = 0; i < 自动乐器_音符数据最大长度; ++i) {
        const float 起音 = (i < 64) ? ((float)(i + 1) / 64.0f) : 1.0f;
        m_包络缓存[i] = 起音;
    }

    // 音符缓存数组 0xFF 条 (实锤)
    m音符缓存数组 = new S音符缓存[0xFF];

    // mp3 解码器 (sub_400E499C → 桩)
    m_mp3_decoder = 创建mp3解码器桩();

    m目录              = nullptr;
    m歌曲数据          = nullptr;
    m歌曲长度          = 0;
    m格式              = 0;
    m_音量保持缓存_index = 0;
    m_to_pwm_index       = 0;
    m当前音符索引      = -1;
    m音符数量          = 0;
}

void CAutoPitch::释放()
{
    delete[] m_音量保持缓存; m_音量保持缓存 = nullptr;
    delete[] m_音量衰减缓存; m_音量衰减缓存 = nullptr;
    delete[] m_包络缓存;     m_包络缓存     = nullptr;
    delete[] m音符缓存数组;  m音符缓存数组  = nullptr;
    if (m_mp3_decoder != nullptr) {
        std::free(m_mp3_decoder);
        m_mp3_decoder = nullptr;
    }
    if (m歌曲数据 != nullptr) {
        delete[] m歌曲数据;
        m歌曲数据 = nullptr;
    }
    m目录     = nullptr;
    m歌曲长度 = 0;
    m格式     = 0;
    m_音量保持缓存_index = 0;
    m_to_pwm_index       = 0;
    m当前音符索引 = -1;
    m音符数量     = 0;
}

// 音量包络 (◐): 起音查 m_包络缓存 表 (64 样本线性爬升) × 尾音线性淡出
float CAutoPitch::包络值(int 位置, int 数据长度)
{
    float 起音 = 1.0f;
    if (m_包络缓存 != nullptr) {
        int 位 = 位置;
        if (位 < 0)
            位 = 0;
        if (位 >= 自动乐器_音符数据最大长度)
            位 = 自动乐器_音符数据最大长度 - 1;
        起音 = m_包络缓存[位];
    } else {
        起音 = (位置 < 64) ? ((float)(位置 + 1) / 64.0f) : 1.0f;
    }
    const int   剩余 = 数据长度 - 位置;
    const float 尾音 = (剩余 < 256) ? ((float)剩余 / 256.0f) : 1.0f;
    return 起音 * 尾音;
}

bool CAutoPitch::载入歌曲文件(const void* 数据, int 长度)
{
    if (数据 == nullptr || 长度 < (int)(k音频区偏移 + k目录条目长度))
        return false;
    SLG_ASSERT(m音符缓存数组 != nullptr);   // 须先 初始化()

    const uint8_t* 文件 = (const uint8_t*)数据;

    // ---- BE 头校验 (校验串实锤: magic/版本 1.0/采样率/声道==1/格式串) ----
    if (读BE32(文件) != slg::kSongFileMagic)
        return false;
    if (std::memcmp(文件 + 4, "1.0", 3) != 0)
        return false;
    const uint32_t 采样率 = 读BE32(文件 + 12);
    const uint32_t 声道   = 读BE32(文件 + 16);
    if (采样率 != (uint32_t)slg::kSampleRate || 声道 != 1)
        return false;
    // 格式串: 头部 64B 内查找 (偏移 ◐)。
    // 注意: 头部含零填充 (如 "1.0" 后), 不能用 strstr —— 须按字节区间搜索
    auto 头内含 = [文件](const char* 串, size_t 串长) {
        for (size_t i = 0; i + 串长 <= 64; ++i)
            if (std::memcmp(文件 + i, 串, 串长) == 0)
                return true;
        return false;
    };
    bool 是mp3 = 头内含("mp3", 3);
    if (!是mp3 && !头内含("raw_short", 9))
        return false;   // 校验串只有 raw_short/mp3 两种
    m格式 = 是mp3 ? 1 : 0;

    // ---- 载入副本 ( songs 可能较大, 一次性拷入堆) ----
    auto* 副本 = new uint8_t[长度];
    std::memcpy(副本, 数据, (size_t)长度);
    if (m歌曲数据 != nullptr)
        delete[] m歌曲数据;
    m歌曲数据 = 副本;
    m歌曲长度 = 长度;
    m目录     = 副本 + k目录偏移;

    // ---- 目录装载: @0x1000 起每条 24B, 直到上限/无效条目 ----
    m音符数量 = 0;
    for (uint32_t i = 0; i < k目录条目上限; ++i) {
        const uint8_t* 条目 = m目录 + i * k目录条目长度;
        const uint32_t 偏移 = 读BE32(条目 + 0);
        const uint32_t 条长 = 读BE32(条目 + 4);
        if (条长 == 0)
            break;   // 终止条目
        if (偏移 < k音频区偏移 || 偏移 + 条长 > (uint32_t)长度)
            break;   // 越界即停 (目录扇区上限语义)
        ++m音符数量;
    }
    return m音符数量 > 0;
}

bool CAutoPitch::解码音符并放入缓存(int 音符索引)
{
    SLG_ASSERT(m目录 != nullptr && m音符缓存数组 != nullptr);
    if (m目录 == nullptr || m音符缓存数组 == nullptr)
        return false;
    if (音符索引 < 0 || 音符索引 >= m音符数量)
        return false;

    const uint8_t* 条目 = m目录 + (uint32_t)音符索引 * k目录条目长度;
    const uint32_t 偏移 = 读BE32(条目 + 0);
    const uint32_t 条长 = 读BE32(条目 + 4);
    if (条长 == 0 || 偏移 < k音频区偏移 || 偏移 + 条长 > (uint32_t)m歌曲长度)
        return false;

    S音符缓存& 缓存 = m音符缓存数组[音符索引];

    if (m格式 == 1) {
        // mp3: 解码器本体未还原 (sub_400E499C 桩) —— TODO 实机补全
        std::printf("CAutoPitch::解码音符并放入缓存 mp3 decoder TODO (sub_400E499C 未还原)\n");
        return false;
    }

    // raw_short: s16LE → float (协议大端与音频小端并存, 录音路径同款)
    const int 样本数 = (int)(条长 / 2);
    auto* 波形 = new float[样本数];
    const uint8_t* 源 = m歌曲数据 + 偏移;
    for (int i = 0; i < 样本数; ++i) {
        const uint16_t u = (uint16_t)源[2 * i] | ((uint16_t)源[2 * i + 1] << 8);
        const int16_t  s = (int16_t)u;
        波形[i] = (float)s / 32768.0f;
    }

    if (缓存.波形 != nullptr)
        delete[] 缓存.波形;
    缓存.波形     = 波形;
    缓存.数据长度 = 样本数;
    缓存.播放位置 = 0;
    缓存.音符索引 = 音符索引;
    缓存.有效     = true;
    m当前音符索引 = 音符索引;
    return true;
}

void CAutoPitch::Add(S音符缓存& 音符, float 音量)
{
    if (!音符.有效 || 音符.波形 == nullptr)
        return;
    SLG_ASSERT(m_音量保持缓存 != nullptr);

    int 剩余数据长度 = 音符.数据长度 - 音符.播放位置;
    int 置零长度     = 0;
    if (剩余数据长度 > 自动乐器_音符数据最大长度) {
        置零长度     = 0;
        剩余数据长度 = 自动乐器_音符数据最大长度;   // 截断到单块上限
        // 固件 printf 实锤格式串 (原样保留)
        std::printf(
            "CAutoPitch::Add m_音量保持缓存_index=%lld, m_to_pwm_index=%lld "
            "剩余数据长度=%d 置零长度=%d > 自动乐器_音符数据最大长度=%d\n",
            (long long)m_音量保持缓存_index, (long long)m_to_pwm_index,
            音符.数据长度 - 音符.播放位置, 置零长度, 自动乐器_音符数据最大长度);
    } else {
        置零长度 = 自动乐器_音符数据最大长度 - 剩余数据长度;
    }

    // ---- 渲染: 音符 PCM × 音量 × 包络 → m_音量保持缓存 (环) ----
    const int64_t 基 = m_音量保持缓存_index % 自动乐器_音符数据最大长度;
    for (int i = 0; i < 剩余数据长度; ++i) {
        const int 源位 = 音符.播放位置 + i;
        const float 采样 = 音符.波形[源位];
        const float 包 = 包络值(源位, 音符.数据长度);
        const int64_t 落 = (基 + i) % 自动乐器_音符数据最大长度;
        m_音量保持缓存[落] = 采样 * 音量 * 包;
    }
    for (int i = 0; i < 置零长度; ++i) {
        const int64_t 落 = (基 + 剩余数据长度 + i) % 自动乐器_音符数据最大长度;
        m_音量保持缓存[落] = 0.0f;
    }

    音符.播放位置 += 剩余数据长度;
    m_音量保持缓存_index += (int64_t)(剩余数据长度 + 置零长度);
}

int CAutoPitch::读取输出(float* 输出, int n)
{
    if (输出 == nullptr || m_音量保持缓存 == nullptr || n <= 0)
        return 0;
    int 实读 = 0;
    for (int i = 0; i < n; ++i) {
        const int64_t 源 = m_to_pwm_index % 自动乐器_音符数据最大长度;
        输出[i] = m_音量保持缓存[源];
        ++m_to_pwm_index;
        ++实读;
    }
    return 实读;
}
