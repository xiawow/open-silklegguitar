// ============================================================================
// BluetoothCustomService.cpp —— 私有 GATT 服务实现 (HRS demo 骨架)
// ----------------------------------------------------------------------------
// 属性表 = 心率 demo 骨架 (4 项) 换 UUID: 0x9578/0x6710/0x2902。
// ESP-IDF v5 Bluedroid API; 骨架保留标准事件流, 细节 TODO 标注。
// ============================================================================

#include "BluetoothCustomService.h"
#include "../JsInterface.h"

#include <cstdio>
#include <cstring>

#if defined(__has_include)
#  if __has_include(<esp_gatts_api.h>)
#    define SLG_HAS_BT 1
#    include "esp_gatts_api.h"
#    include "esp_gap_ble_api.h"
#    include "esp_bt_main.h"
#  endif
#endif

bool CBluetoothCustomService::s_connected = false;
int  CBluetoothCustomService::s_mtu       = 23;

#ifdef SLG_HAS_BT

// ---- 属性表 (HRS demo 4 项骨架: svc / decl / val / cccd) --------------------
enum { IDX_SVC = 0, IDX_CHR_DECL, IDX_CHR_VAL, IDX_CHR_CCCD, IDX_NB };

static const uint16_t kSvcUuid  = 0x9578;   // 前端 _8389
static const uint16_t kCharUuid = 0x6710;   // 前端 _8390
static const uint16_t kCccdUuid = 0x2902;   // HRS demo 保留

// 16-bit UUID 公共前缀 (蓝牙 Base UUID)
static const uint8_t kUuidPrefix[12] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00
};

// TODO: 与原固件逐项核对权限 (RE 只确认了 indicate+write 属性与 CCCD 存在)
static const esp_gatts_attr_db_t kAttrTable[IDX_NB] = {
    // 服务声明
    [IDX_SVC] = {
        {.attr_control = {.auto_rsp = ESP_GATT_AUTO_RSP},
         .attr_uuid = {.len = ESP_UUID_LEN_16, .uuid = {.uuid16 = ESP_GATT_UUID_PRI_SERVICE}}},
        {.attr_max_len = 2, .attr_len = 2,
         .attr_value = (uint8_t*)&kSvcUuid},
    },
    // 特征声明 (indicate + write)
    [IDX_CHR_DECL] = {
        {.attr_control = {.auto_rsp = ESP_GATT_AUTO_RSP},
         .attr_uuid = {.len = ESP_UUID_LEN_16, .uuid = {.uuid16 = ESP_GATT_UUID_CHAR_DECLARE}}},
        {.attr_max_len = 1, .attr_len = 1,
         .attr_value = (uint8_t[]){
             ESP_GATT_CHAR_PROP_BIT_INDICATE | ESP_GATT_CHAR_PROP_BIT_WRITE}},
    },
    // 特征值 (写入口; 指示数据从 GATTS_WRITE/CONFIRM 事件走)
    [IDX_CHR_VAL] = {
        {.attr_control = {.auto_rsp = ESP_GATT_RSP_BY_APP},
         .attr_uuid = {.len = ESP_UUID_LEN_16, .uuid = {.uuid16 = kCharUuid}}},
        {.attr_max_len = 512, .attr_len = 0, .attr_value = nullptr},
    },
    // CCCD (HRS demo 保留; 指示开关由前端订阅行为控制)
    [IDX_CHR_CCCD] = {
        {.attr_control = {.auto_rsp = ESP_GATT_AUTO_RSP},
         .attr_uuid = {.len = ESP_UUID_LEN_16, .uuid = {.uuid16 = kCccdUuid}}},
        {.attr_max_len = 2, .attr_len = 2, .attr_value = (uint8_t[]){0, 0}},
    },
};

void CBluetoothCustomService::Init()
{
    // 由 BluetoothManager 的 ESP_GATTS_REG_EVT 里调用:
    // esp_ble_gatts_create_attr_tab(kAttrTable, SLG_APP_ID, IDX_NB, 0);
    // TODO: handle 表回调 (ESP_GATTS_CREAT_ATTR_TAB_EVT) 保存 handle
    printf("CBluetoothCustomService::Init (attr tab creation pending via reg evt)\n");
}

bool CBluetoothCustomService::SendIndicate(const void* data, int len)
{
    if (data == nullptr || len <= 0 || len > 509) {   // 固件断言 ≤0x1FD
        SLG_ASSERT(0);
        return false;
    }
    if (!s_connected)
        return false;
    // TODO: 保存 gatts_if/conn_id (事件回调填), 然后:
    // esp_ble_gatts_send_indicate(gatts_if, conn_id, h_val,
    //                             len, (uint8_t*)data, true /*need_confirm*/);
    (void)kUuidPrefix;
    return true;   // 骨架: 假装成功 (避免宿主/未连接路径空转)
}

void CBluetoothCustomService::OnConnect()
{
    s_connected = true;
    // TODO: esp_ble_gatt_set_local_mtu(512) 在栈初始化时做; 这里触发交换
}

void CBluetoothCustomService::OnDisconnect() { s_connected = false; }

void CBluetoothCustomService::OnWrite(const uint8_t* v, int len)
{
    // 原固件: OnBluetoothDataCallback @0x400DC834 = memcpy 静态缓冲 → Drive
    CJsInterface::OnBluetoothData(v, len);
    CJsInterface::Drive();
}

void CBluetoothCustomService::OnMtuChanged(int mtu)
{
    s_mtu = mtu;
    // 协议帧上限恒 509 (JS _8134 硬编码), MTU 仅影响底层单次 WRITE 容量
}

#else  // !SLG_HAS_BT —— 宿主端/无 ESP 环境

void CBluetoothCustomService::Init() {}
bool CBluetoothCustomService::SendIndicate(const void*, int) { return false; }
void CBluetoothCustomService::OnConnect()    { s_connected = true; }
void CBluetoothCustomService::OnDisconnect() { s_connected = false; }
void CBluetoothCustomService::OnWrite(const uint8_t* v, int len)
{
    CJsInterface::OnBluetoothData(v, len);
    CJsInterface::Drive();
}
void CBluetoothCustomService::OnMtuChanged(int mtu) { s_mtu = mtu; }

#endif // SLG_HAS_BT
