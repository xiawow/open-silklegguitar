// ============================================================================
// JsInterface.h —— BLE 命令协议层 (对应前端 _27.js class _1170 的对端)
// ----------------------------------------------------------------------------
// 还原来源: 固件 CJsInterface::Drive() @0x400DCBE8 (strcmp 分发链)
//           + OnBluetoothDataCallback @0x400DC834
//           + 前端 _27.js 完整命令表交叉验证 (25 上行 / 16 下行)
// 置信度:   ★★★★★ 命令名/负载格式全部经 JS 实证; 个别处理体内部逻辑为桩 (标 TODO)
// ============================================================================
#pragma once

#include "Common.h"
#include "BinaryBuffer.h"

class CJsInterface {
public:
    // BLE WRITE 事件 → 存入静态缓冲 (固件: memcpy 静态区 + 记长度, 原 JS 桥行为)
    static void OnBluetoothData(const void* data, int len);

    // 解析静态缓冲: [u32 魔数][string 命令名][负载] → strcmp 分发执行
    // RE: "Error CJsInterface::Drive received not ..." 为未知命令日志
    static bool Drive();

    // ---- 响应发送 (下行包 = [string 命令名][u32 total][u32 pos][u32 len][data],
    //      >509 自动分帧, JS _1191 按同格式重组; 两遍构建) --------------------
    static void SendResponse(const char* cmd, const void* payload, int total);
    static void SendResponseString(const char* cmd, const char* text);

    // ---- 供处理桩调用的管理器接口 (实现见各 .cpp, 骨架内多为桩) -----------
    // 这些在骨架中直接调用对应类, 见 JsInterface.cpp 各 case

private:
    static uint8_t s_rxBuffer[512];
    static int     s_rxLength;

    // 单帧指示发送 → driver/BluetoothCustomService.cpp (assert ≤509, need_confirm=1)
    static bool SendIndicateFrame(const void* data, int len);
};
