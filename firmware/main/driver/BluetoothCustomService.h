// ============================================================================
// BluetoothCustomService.h —— 私有 GATT 服务 (协议数据通道)
// ----------------------------------------------------------------------------
// 出处 (RE 实锤): 固件 assert 字符串含 HRS_IDX_NB → 由 ESP-IDF 心率测量
// GATT Server demo 改造; 原 0x180D/0x2A37 替换为私有 UUID, 保留 CCCD 0x2902,
// 属性表骨架 4 项 (svc / char 声明 / char 值 / CCCD)。
//   svc  = 0x9578  (前端 index.html _8389 过滤)
//   char = 0x6710  (indicate + write, 前端 _8390)
// 数据流: WRITE_EVT → CJsInterface::OnBluetoothData → Drive()
//         JsInterface → SendIndicateFrame → 本类 SendIndicate (≤509, confirm=1)
// ============================================================================
#pragma once

#include "Common.h"

class CBluetoothCustomService {
public:
    static void Init();     // 注册属性表 (GATTS app 注册后由 Manager 事件驱动)

    // 下行单帧指示 (JS _8158.writeValue 对端)。len > 509 断言 (固件原始行为)。
    // 返回 false = 未连接或栈未就绪。
    static bool SendIndicate(const void* data, int len);

    static bool IsConnected() { return s_connected; }

    // ---- 由 BluetoothManager 的事件回调转发 --------------------------------
    static void OnConnect();                       // 置位 + 默认 MTU 请求
    static void OnDisconnect();                    // 清位
    static void OnWrite(const uint8_t* v, int len); // → OnBluetoothData + Drive
    static void OnMtuChanged(int mtu);             // 记录协商 MTU (≤512)

private:
    static bool s_connected;
    static int  s_mtu;          // 协商结果 (默认 23, JS 侧请求大 MTU)
};
