// ============================================================================
// I2S.cpp —— 音频驱动桩 (IDF v5 i2s_chan API 接线 TODO)
// ============================================================================
#include "I2S.h"

#include <cstdio>
#include <cstring>

int     CI2sDriver::s_rate     = 16000;
ESource CI2sDriver::s_channels = ESource::mono;

void CI2sDriver::Init(int sampleRate, ESource channels)
{
    s_rate = sampleRate;
    s_channels = channels;
    // TODO(IDF v5): i2s_new_channel(TX+RX) → i2s_channel_init_std_mode
    // (std 配置: 16kHz / 16bit / mono|stereo, GPIO 按硬件 TODO) → enable
    // ADC 输入路径: Adc1Dma 头文件式组件, 电压档位 EADC1VoltageRange TODO
    printf("CI2sDriver::Init %dHz %s (stub)\n", sampleRate,
           channels == ESource::stereo ? "stereo" : "mono");
}

bool CI2sDriver::ReadMono(int16_t* out, int samples)
{
    // TODO: i2s_channel_read (或 Adc1Poll 轮询路径)
    if (out != nullptr)
        memset(out, 0, samples * 2);
    return true;
}

bool CI2sDriver::WriteStereo(const int16_t* l, const int16_t* r, int samples)
{
    // TODO: i2s_channel_write (mono 输出设备取 l 通道)
    (void)l; (void)r; (void)samples;
    return true;
}
