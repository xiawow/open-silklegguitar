// ============================================================================
// app_main —— 启动链
// ----------------------------------------------------------------------------
// 横幅 ★ 必须原样: "Dashichang silk leg guita(version: 2.0) starting...\n"
// (固件 rodata @0x4C14; 前端 _8272 对 OTA 包 _710.bin 用同一串做版本解析)
// 初始化顺序按 RE 推断: 横幅 → NVS → 配置 → BT → 音频 → 业务
// ============================================================================
#include "Common.h"
#include "UserConfigure.h"
#include "SilkLegGuitar.h"
#include "PickupPlugin.h"
#include "RecordTemporarily.h"
#include "driver/BluetoothManager.h"
#include "driver/A2DP.h"
#include "driver/I2S.h"

#if defined(__has_include)
#  if __has_include(<freertos/FreeRTOS.h>)
#    include "freertos/FreeRTOS.h"
#    include "freertos/task.h"
#    include "nvs_flash.h"
#    define SLG_ON_ESP 1
#  endif
#endif

#include <cstdio>

extern "C" void app_main(void)
{
    printf("%s", slg::kStartBanner);          // ★ 原样, 勿改

#ifdef SLG_ON_ESP
    // NVS (BT 栈依赖)
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
#endif

    CUserConfigure::Init();                    // module/output (缺省 line_out)
    CSilkLegGuitar::Init();                    // GPIO + 恢复配置
    CPickupPlugin::Init();
    CRecordTemporarily::Init();

    CBluetoothManager::Init();                 // controller(BTDM) → GATT 0x9578 广播
    CA2dpSource::Init();                       // 音箱输出路径
    CI2sDriver::Init(slg::kSampleRate, ESource::mono);   // 16kHz 麦克风链

#ifdef SLG_ON_ESP
    // TODO 主循环: 音频泵 (I2S → AutoPitch/拾音 → 输出) + 插件轮询
    while (true)
        vTaskDelay(pdMS_TO_TICKS(100));
#else
    printf("skeleton main loop (host build)\n");
#endif
}
