// ============================================================================
// BluetoothManager.h —— BT 栈生命周期 + 广播 + 音箱状态机
// ----------------------------------------------------------------------------
// 需要 BLE (GATT) + 经典蓝牙 (A2DP 音箱) 双模 → controller BTDM 模式。
// 音箱状态串与前端 _8150 严格一致 (拼写 "not_initialzied" 为固件原生)。
// ============================================================================
#pragma once

#include "Common.h"

class CBluetoothManager {
public:
    enum class SpeakerState {
        not_initialized,     // 固件原生拼写 (前端 case 同名依赖, 勿改)
        free,
        searching_speaker,
        connecting,
        connected,
    };

    static void Init();      // controller(BTDM) → bluedroid → GATTS 注册 → 广播

    static bool IsReady()    { return s_ready; }

    // 音箱操作 (前端 Search / BluetoothDriveSpeakerConnect / ...Disconnect 对端)
    static void StartSearch();        // 经典蓝牙设备发现 → 逐台 Discovered 响应
    static void StopSearch();
    static void ConnectSpeaker(const uint8_t mac[6], const char* name30);
    static void DisconnectSpeaker();

    static SpeakerState GetSpeakerState() { return s_speaker; }

    // 状态串 (固件字符串区原文; connected 时响应里另附 6B MAC + 30B 名)
    static const char* GetSpeakerStateString();

private:
    static bool          s_ready;
    static SpeakerState  s_speaker;
    static uint8_t       s_speakerMac[6];
    static char          s_speakerName[30];
};
