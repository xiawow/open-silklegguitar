// ============================================================================
// UserConfigure.h —— Flash 配置存取 (TLV 序列化)
// ----------------------------------------------------------------------------
// 还原来源: 前端 _27.js _8143 (UserConfigure 响应解析) 逐字段对照 + 固件
//           LE 常量 0xE45B901F @0x40FE8 (构造响应时使用同一魔数)
// TLV 条目: [u32 BE 0xE45B901F][string 键][i32 BE 大小][原始值] 逐条拼接
// 已知键 (★值均为字符串, JS _8201/_1105 读取):
//   "module" —— "auto_pitch" / "pickup" / "midi_device"
//   "output" —— 5 个 EOutput 名, 缺省 "line_out" (JS _8143 显式缺省)
// ============================================================================
#pragma once

#include "Common.h"

class CUserConfigure {
public:
    // 载入持久层 (骨架: 内存缺省; TODO: NVS 读写)
    // 缺省: module="pickup"(◐ 顺序存疑) / output="line_out"(JS 实证)
    static void Init();

    static bool GetString(const char* key, char* out, int cap);
    static bool SetString(const char* key, const char* value);

    // 序列化为 TLV 流 (大端直写 out), 返回字节数; 放不下返回 0
    static int  Serialize(uint8_t* out, int maxLen);

    // 从 TLV 流恢复 (保留给 SetUserConfigure 类命令 / 启动加载)
    static bool Deserialize(const void* data, int len);

    // TODO: 持久化落盘 (原固件写 flash 分区, 分区名 RE 未反出)
    static void Commit();

private:
    // 骨架: 定长键值表。原固件结构 RE 未完整反出, ◐ 仅保证对外格式一致
    struct Entry {
        bool used;
        char key[24];
        char value[32];
    };
    static Entry s_entries[8];
    static int   FindEntry(const char* key);
    static int   AllocEntry(const char* key);
};
