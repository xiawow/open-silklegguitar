// ============================================================================
// BluetoothManager.cpp —— BT 栈初始化链 + 状态机
// ----------------------------------------------------------------------------
// 初始化链 (标准 Bluedroid 流程): controller init/enable(BTDM) →
// bluedroid init/enable → GATTS register → GAP 广播。骨架保留流程与事件桩。
// ============================================================================

#include "BluetoothManager.h"
#include "BluetoothCustomService.h"

#include <cstdio>
#include <cstring>

#if defined(__has_include)
#  if __has_include(<esp_bt.h>)
#    define SLG_HAS_BT 1
#    include "esp_bt.h"
#    include "esp_bt_main.h"
#    include "esp_gap_ble_api.h"
#    include "esp_gatts_api.h"
#  endif
#endif

bool         CBluetoothManager::s_ready  = false;
CBluetoothManager::SpeakerState CBluetoothManager::s_speaker =
    CBluetoothManager::SpeakerState::not_initialized;
uint8_t      CBluetoothManager::s_speakerMac[6]  = {0};
char         CBluetoothManager::s_speakerName[30] = {0};

// ---------------------------------------------------------------------------

const char* CBluetoothManager::GetSpeakerStateString()
{
    // 拼写与固件字符串区一致 (前端 _8150 switch 逐字匹配)
    switch (s_speaker) {
        case SpeakerState::not_initialized:   return "not_initialzied";
        case SpeakerState::free:              return "free";
        case SpeakerState::searching_speaker: return "searching_speaker";
        case SpeakerState::connecting:        return "connecting";
        case SpeakerState::connected:         return "connected";
    }
    return "not_initialzied";
}

#ifdef SLG_HAS_BT

void CBluetoothManager::Init()
{
    // TODO(事件接线): GATTS_APP_REGISTER_EVT → CBluetoothCustomService::Init +
    //                 esp_ble_gap_set_device_name / config adv;
    //                 GAP BLE 事件 → OnConnect/OnDisconnect/OnWrite/OnMtuChanged。
    // 广播名 = 蓝牙设备名 (前端 requestDevice filters 按名过滤; 该名未转义
    // 拼进 onclick 属性 —— 见逆向报告 JS 注入点)
    printf("CBluetoothManager::Init (controller/bluedroid chain TODO wire-up)\n");
    s_ready = true;
}

void CBluetoothManager::StartSearch()
{
    s_speaker = SpeakerState::searching_speaker;
    // TODO: esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, ...)
    // 每台设备 → 组 BluetoothDriveSpeakDiscovered 帧 (6B MAC + 30B 名裸字节)
    // 经 JsInterface SendResponse 发出 (JS onclick 注入点源数据)
    printf("CBluetoothManager::StartSearch (discovery TODO)\n");
}

void CBluetoothManager::StopSearch()
{
    // TODO: esp_bt_gap_cancel_discovery
    if (s_speaker == SpeakerState::searching_speaker)
        s_speaker = SpeakerState::free;
}

void CBluetoothManager::ConnectSpeaker(const uint8_t mac[6], const char* name30)
{
    s_speaker = SpeakerState::connecting;
    if (mac != nullptr) memcpy(s_speakerMac, mac, 6);
    if (name30 != nullptr) { memcpy(s_speakerName, name30, 30); s_speakerName[29] = 0; }
    // TODO: esp_a2d_source_connect(s_speakerMac) → 连接事件改 connected
    printf("CBluetoothManager::ConnectSpeaker (a2d TODO)\n");
}

void CBluetoothManager::DisconnectSpeaker()
{
    // TODO: esp_a2d_source_disconnect
    s_speaker = SpeakerState::free;
}

#else  // !SLG_HAS_BT

void CBluetoothManager::Init() { s_ready = true; }
void CBluetoothManager::StartSearch() { s_speaker = SpeakerState::searching_speaker; }
void CBluetoothManager::StopSearch()  { if (s_speaker == SpeakerState::searching_speaker) s_speaker = SpeakerState::free; }
void CBluetoothManager::ConnectSpeaker(const uint8_t mac[6], const char* name30)
{
    s_speaker = SpeakerState::connecting;
    if (mac != nullptr) memcpy(s_speakerMac, mac, 6);
    if (name30 != nullptr) { memcpy(s_speakerName, name30, 30); s_speakerName[29] = 0; }
}
void CBluetoothManager::DisconnectSpeaker() { s_speaker = SpeakerState::free; }

#endif // SLG_HAS_BT
