// ============================================================================
// PickupPlugin.cpp —— 插件格式解析 (按 JS _8286 完整实现) + 加载桩
// ----------------------------------------------------------------------------
// 解析/重定位/对齐逻辑与前端逐字段同源 (★★★★★);
// "加载执行" (mmap + 跳转) 是骨架桩 —— 原固件该部分 RE 未反出可安全描述的
// 细节, 且真执行属固件安全边界 (见逆向报告安全发现: 无签名/无魔数校验)。
// ============================================================================

#include "PickupPlugin.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// 插件总大小上限 800KB (RE: JS 侧 800KB×1024 校验), 代码/数据段各 ≤128KB
namespace {
constexpr int kPluginMaxSize    = 800 * 1024;
constexpr int kSegmentMaxSize   = 128 * 1024;
constexpr int kAlignBoundary    = 64 * 1024;
constexpr uint8_t kFillByte     = 0xCC;    // int3 断点填充 (前端实证)
}

uint8_t* CPickupPlugin::s_image     = nullptr;
int      CPickupPlugin::s_size      = 0;
int      CPickupPlugin::s_capacity  = 0;
bool     CPickupPlugin::s_parsed    = false;

// ---------------------------------------------------------------------------

void CPickupPlugin::Init()
{
    free(s_image);
    s_image = nullptr;
    s_size = 0;
    s_capacity = 0;
    s_parsed = false;
}

uint32_t CPickupPlugin::GetCodeBase() { return 0x400E0000; }   // TODO: 实机布局
uint32_t CPickupPlugin::GetDataBase() { return 0x3FFC0000; }   // TODO: 实机布局

bool CPickupPlugin::OnDownloadData(uint32_t offset, const uint8_t* data, int len)
{
    if (data == nullptr || len <= 0 || offset > (uint32_t)kPluginMaxSize)
        return false;
    if ((int)offset + len > kPluginMaxSize)
        return false;

    if (s_image == nullptr || (int)offset + len > s_capacity) {
        // 按需扩容 (首个分片不携带总长, 指数扩容足够)
        int need = (int)offset + len;
        int cap  = (s_capacity > 0) ? s_capacity : 64 * 1024;
        while (cap < need)
            cap *= 2;
        uint8_t* p = (uint8_t*)realloc(s_image, cap);
        if (p == nullptr) {
            printf("Error CPickupPlugin alloc %d\n", cap);
            return false;
        }
        s_image = p;
        s_capacity = cap;
    }
    memcpy(s_image + offset, data, len);
    if ((int)offset + len > s_size)
        s_size = (int)offset + len;
    return true;
}

// ---------------------------------------------------------------------------
// 头解析 —— 与 _8286 逐字段对照 (头整数全小端!)
// ---------------------------------------------------------------------------
bool CPickupPlugin::FinalizeDownload()
{
    s_parsed = false;
    if (s_image == nullptr || s_size < 4)
        return false;

    // 小端读取器 (与协议层大端读取器相反 —— 插件头是 Xtensa 原生 LE)
    auto rdU32LE = [&](int off) -> uint32_t {
        return (uint32_t)s_image[off] |
               ((uint32_t)s_image[off + 1] << 8) |
               ((uint32_t)s_image[off + 2] << 16) |
               ((uint32_t)s_image[off + 3] << 24);
    };

    int off = 0;
    const uint32_t magic = rdU32LE(off); off += 4;
    // ★ 魔数仅前端校验, 固件原版不查 (安全发现) —— 骨架保留检查但仅告警
    if (magic != slg::kPickupPluginMagic)
        printf("Warning CPickupPlugin magic 0x%08X != 0x%08X (firmware never checks!)\n",
               magic, slg::kPickupPluginMagic);

    const uint32_t f[5] = { rdU32LE(off), rdU32LE(off + 4), rdU32LE(off + 8),
                            rdU32LE(off + 12), rdU32LE(off + 16) };   // 版本/入口等 ◐
    off += 20;
    (void)f;

    const uint32_t symOff  = rdU32LE(off);  const uint32_t symSize  = rdU32LE(off + 4);  off += 8;
    const uint32_t codeOff = rdU32LE(off);  const uint32_t codeSize = rdU32LE(off + 4);  off += 8;
    if (codeSize > (uint32_t)kSegmentMaxSize || codeOff + codeSize > (uint32_t)s_size) {
        printf("Error CPickupPlugin code segment 0x%x+0x%x\n", codeOff, codeSize);
        return false;
    }

    // 代码段两组重定位: n1 条 += 数据段基址; n2 条 += 代码段基址
    const uint32_t codeBase = GetCodeBase();
    const uint32_t dataBase = GetDataBase();
    for (int g = 0; g < 2; ++g) {
        const uint32_t n = rdU32LE(off); off += 4;
        if (off + (int)(n * 4) > s_size) {
            printf("Error CPickupPlugin code reloc overflow\n");
            return false;
        }
        for (uint32_t i = 0; i < n; ++i) {
            const uint32_t rel = rdU32LE(off); off += 4;
            if (rel + 4 > codeSize)
                continue;                     // 越界重定位条目跳过 (前端同语义◐)
            const int at = (int)codeOff + (int)rel;
            // 原地加基址: 镜像内已是 LE, 直接小端加
            const uint32_t w = rdU32LE(at);
            const uint32_t fixed = w + (g == 0 ? dataBase : codeBase);
            s_image[at]     = (uint8_t)(fixed);
            s_image[at + 1] = (uint8_t)(fixed >> 8);
            s_image[at + 2] = (uint8_t)(fixed >> 16);
            s_image[at + 3] = (uint8_t)(fixed >> 24);
        }
    }

    const uint32_t dataOff = rdU32LE(off);  const uint32_t dataSize = rdU32LE(off + 4);  off += 8;
    if (dataSize > (uint32_t)kSegmentMaxSize || dataOff + dataSize > (uint32_t)s_size) {
        printf("Error CPickupPlugin data segment 0x%x+0x%x\n", dataOff, dataSize);
        return false;
    }

    // 数据段: 一个 u32 字段 + 两组重定位 (+= 数据段基址 / 代码段基址)
    const uint32_t df = rdU32LE(off); off += 4; (void)df;
    for (int g = 0; g < 2; ++g) {
        const uint32_t n = rdU32LE(off); off += 4;
        if (off + (int)(n * 4) > s_size) {
            printf("Error CPickupPlugin data reloc overflow\n");
            return false;
        }
        for (uint32_t i = 0; i < n; ++i) {
            const uint32_t rel = rdU32LE(off); off += 4;
            if (rel + 4 > dataSize)
                continue;
            const int at = (int)dataOff + (int)rel;
            const uint32_t w = rdU32LE(at);
            const uint32_t fixed = w + (g == 0 ? dataBase : codeBase);
            s_image[at]     = (uint8_t)(fixed);
            s_image[at + 1] = (uint8_t)(fixed >> 8);
            s_image[at + 2] = (uint8_t)(fixed >> 16);
            s_image[at + 3] = (uint8_t)(fixed >> 24);
        }
    }

    // 代码段向前对齐 64KB, 0xCC 填充 (前端 _8286 实证; 检查镜像内预留区)
    const uint32_t aligned = (codeOff / kAlignBoundary) * kAlignBoundary;
    if (aligned < codeOff) {
        for (uint32_t i = aligned; i < codeOff; ++i)
            if (s_image[i] != kFillByte && s_image[i] != 0)
                printf("Warning CPickupPlugin align gap byte 0x%02X @0x%x\n", s_image[i], i);
    }

    s_parsed = true;
    // TODO: 原固件 mmap 到可执行地址并调用入口 (f[1]?) —— 骨架不执行
    printf("CPickupPlugin parsed: code 0x%x+0x%x data 0x%x+0x%x sym 0x%x+0x%x\n",
           codeOff, codeSize, dataOff, dataSize, symOff, symSize);
    return true;
}

// ---------------------------------------------------------------------------
// 宿主 HTML (GetPickupPluginHtml 源) —— 骨架内嵌最小页
// 原固件的插件 HTML 存于固件数据区 (RE: 同源 blob iframe 反向控制通道)
// ---------------------------------------------------------------------------
const uint8_t* CPickupPlugin::GetHtml(int* len)
{
    static const char* kHtml =
        "<!DOCTYPE html><html><head><meta charset='utf-8'>"
        "<title>Pickup Plugin</title></head>"
        "<body><h3>Pickup plugin placeholder</h3>"
        "<p>This skeleton does not embed the original plugin UI.</p>"
        "</body></html>";
    if (len != nullptr)
        *len = (int)strlen(kHtml);
    return (const uint8_t*)kHtml;
}

bool CPickupPlugin::SendToCpp(const uint8_t* data, int len)
{
    // 运行时插件→固件数据通道 (≤459B/包)。TODO: 语义 RE 未反出
    (void)data;
    printf("CPickupPlugin::SendToCpp %d bytes (stub)\n", len);
    return true;
}

const uint8_t* CPickupPlugin::PollJsOutput(int* len)
{
    // TODO: 插件运行产出 JS → 固件经 PickupPluginSetJs 下发 (轮询点)
    if (len != nullptr)
        *len = 0;
    return nullptr;
}
