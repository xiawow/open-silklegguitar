// ============================================================================
// UserConfigure.cpp —— TLV 配置存取实现
// ----------------------------------------------------------------------------
// 对外格式置信度 ★★★★★ (JS _8143 三种读取器实证: _8201 字符串 / _8202 i32
// (断言 size==4) / _8203 bool (断言 size==1)); module/output 走字符串读取。
// 内部存储结构为骨架推断 ◐。
// ============================================================================

#include "UserConfigure.h"
#include "BinaryBuffer.h"

#include <cstdio>
#include <cstring>

CUserConfigure::Entry CUserConfigure::s_entries[8];

// ---------------------------------------------------------------------------

void CUserConfigure::Init()
{
    for (auto& e : s_entries)
        e.used = false;
    // 缺省值: output 缺省 "line_out" 为 JS _8143 实证; module 缺省 ◐ 推断
    SetString("module", "pickup");
    SetString("output", "line_out");
    // TODO: 从 NVS/flash 分区读回 (键名与原固件一致)
}

int CUserConfigure::FindEntry(const char* key)
{
    for (int i = 0; i < 8; ++i)
        if (s_entries[i].used && strcmp(s_entries[i].key, key) == 0)
            return i;
    return -1;
}

int CUserConfigure::AllocEntry(const char* key)
{
    int i = FindEntry(key);
    if (i < 0)
        for (i = 0; i < 8; ++i)
            if (!s_entries[i].used)
                break;
    if (i >= 8)
        return -1;
    s_entries[i].used = true;
    snprintf(s_entries[i].key, sizeof(s_entries[i].key), "%s", key);
    return i;
}

bool CUserConfigure::GetString(const char* key, char* out, int cap)
{
    const int i = FindEntry(key);
    if (i < 0 || out == nullptr || cap <= 0)
        return false;
    snprintf(out, cap, "%s", s_entries[i].value);
    return true;
}

bool CUserConfigure::SetString(const char* key, const char* value)
{
    if (key == nullptr || value == nullptr)
        return false;
    const int i = AllocEntry(key);
    if (i < 0)
        return false;
    snprintf(s_entries[i].value, sizeof(s_entries[i].value), "%s", value);
    return true;
}

// 条目: [u32 BE 0xE45B901F][string key][i32 size][值字节]
// 用 CBinaryBuffer 两遍构建, 与固件构造响应的方式同源 (常量 LE @0x40FE8 →
// 内存整数, 写线序时 bswap 成大端 —— 与 AppendU32 的 RE 实现一致)
int CUserConfigure::Serialize(uint8_t* out, int maxLen)
{
    if (out == nullptr || maxLen <= 0)
        return 0;

    CBinaryBuffer b;
    bool ok = true;
    for (int pass = 0; pass < 2 && ok; ++pass) {
        if (pass == 0)      b.StartBuild();
        else if (!b.EndBuild()) { SLG_ASSERT(0); return 0; }

        for (const auto& e : s_entries) {
            if (!e.used)
                continue;
            b.AppendU32(slg::kUserConfigureMagic);
            b.AppendString(e.key);
            const int vlen = (int)strlen(e.value);
            b.AppendI32(vlen);
            if (vlen > 0)
                b.AppendBuffer(e.value, vlen);
        }
        ok = b.IsValid();
    }
    if (!ok || b.Length() > maxLen)
        return 0;
    memcpy(out, b.Data(), b.Length());
    return b.Length();
}

bool CUserConfigure::Deserialize(const void* data, int len)
{
    if (data == nullptr || len <= 0)
        return false;
    CBinaryBuffer b;
    b.Attach(data, len);
    while (b.Position() + 8 <= len) {
        const uint32_t magic = b.ReadU32();
        if (magic != slg::kUserConfigureMagic)
            return false;                    // JS _8199: 魔字不对即 false
        char key[24] = {0};
        if (!b.ReadString8(key, (int)sizeof(key)))
            return false;
        const int size = b.ReadI32();
        if (size < 0 || b.Position() + size > len)
            return false;
        char value[32] = {0};
        if (size > 0 && !b.ReadBytes(value, size > 31 ? 31 : size))
            return false;
        SetString(key, value);
    }
    return true;
}

void CUserConfigure::Commit()
{
    // TODO: NVS 写入 (原固件分区名/布局 RE 未反出)
}
