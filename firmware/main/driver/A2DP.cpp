// ============================================================================
// A2DP.cpp —— A2DP Source 桩
// ============================================================================
#include "A2DP.h"

#include <cstdio>

bool CA2dpSource::s_active = false;

void CA2dpSource::Init()
{
    // TODO: esp_a2d_source_init + esp_a2d_register_callback +
    //       esp_a2d_source_register_data_callback + esp_avrc_ct_init
    // (controller 必须为 BTDM 双模 —— 与 GATT 共存)
    printf("CA2dpSource::Init (stub)\n");
}

bool CA2dpSource::Start()
{
    s_active = true;
    return true;
}

void CA2dpSource::Stop() { s_active = false; }

bool CA2dpSource::WritePcm(const int16_t* interleavedStereo, int sampleCount)
{
    // TODO: 填 A2DP 媒体包 (SBC 编码) —— 原固件用 esp-sbc/内置编码器, RE 未反出
    (void)interleavedStereo;
    (void)sampleCount;
    return s_active;
}
