// ============================================================================
// PickupPlugin.h —— 拾音器插件下载/解析/宿主
// ----------------------------------------------------------------------------
// 还原来源:
//   * 前端 _613.js _8286 —— 插件文件格式与重组逻辑 (置信度 ★★★★★)
//   * 固件 PickupPlugin.cpp —— mmap 加载 (成员名 m_code_segment__fix_address
//     等), 骨架中"加载执行"为桩
// 格式 (★头部整数字段为小端 —— Xtensa LE 原生代码, 与协议大端相反):
//   u32 魔数 0x5D9F2A6B          ← 仅前端校验! 固件二进制中不存在此常量
//   u32 ×5                       (版本/入口等, 语义 ◐)
//   u32 符号表偏移 / 大小
//   u32 代码段偏移 / 大小        (≤128KB)
//   u32 重定位条数 n1 + n1×u32   (代码字 += 数据段基址)
//   u32 重定位条数 n2 + n2×u32   (代码字 += 代码段基址)
//   u32 数据段偏移 / 大小        (≤128KB)
//   u32 + 两组数据段重定位       (+= 数据段基址 / 代码段基址)
//   代码段原始字节、数据段原始字节、符号表
// 前端重组: 重定位加基址 → 代码段向前对齐 64KB (0xCC 填充=int3) → 回填三个
// 最终地址 → 下发镜像。
// ============================================================================
#pragma once

#include "Common.h"

class CPickupPlugin {
public:
    static void Init();

    // 基址 (GetPickupPluginBaseAddress 响应; JS _8146/_8147 存下供重定位。
    // 骨架给固定占位地址 —— 实机值取决于固件 mmap 布局, TODO)
    static uint32_t GetCodeBase();
    static uint32_t GetDataBase();

    // DownloadPickupPlugin 数据汇 (offset = 块×4096 + 片×455 绝对偏移)
    static bool OnDownloadData(uint32_t offset, const uint8_t* data, int len);

    // 全部收完: 解析头 + 应用重定位 + 0xCC 对齐检查 (执行 = mmap 桩)
    static bool FinalizeDownload();

    // 插件宿主 HTML (GetPickupPluginHtml 响应源; JS 渲染进同源 blob iframe
    // —— 固件→页面反向控制通道, 见逆向报告安全发现)。骨架内嵌最小页。
    static const uint8_t* GetHtml(int* len);

    // PickupPluginSendToCpp 运行时通道 (插件↔固件, ≤459B/包; 桩)
    static bool SendToCpp(const uint8_t* data, int len);

    // 插件产出的 JS (PickupPluginSetJs 下行源; 轮询桩, 骨架恒为空)
    static const uint8_t* PollJsOutput(int* len);

private:
    static uint8_t* s_image;      // 下载镜像堆缓冲 (≤800KB, 按需分配)
    static int      s_size;       // 已收字节数
    static int      s_capacity;
    static bool     s_parsed;
};
