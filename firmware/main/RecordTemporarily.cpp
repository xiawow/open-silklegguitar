// ============================================================================
// RecordTemporarily.cpp —— 临时录音缓冲实现
// ============================================================================

#include "RecordTemporarily.h"

#include <cstring>

bool     CRecordTemporarily::s_recording = false;
CRecordTemporarily::Source CRecordTemporarily::s_source = CRecordTemporarily::Source::input;
uint16_t CRecordTemporarily::s_buffer[CRecordTemporarily::kCapacity];
int      CRecordTemporarily::s_count = 0;

// ---------------------------------------------------------------------------

void CRecordTemporarily::Init()
{
    s_recording = false;
    s_count = 0;
}

bool CRecordTemporarily::Start(Source src)
{
    s_source = src;
    s_count = 0;
    s_recording = true;
    // TODO: 接音频泵 (CI2sDriver::ReadMono / A2DP 混音), 实机录满或超时回发
    return true;
}

void CRecordTemporarily::Stop()
{
    s_recording = false;
}

void CRecordTemporarily::Feed(const int16_t* samples, int count)
{
    if (!s_recording || samples == nullptr || count <= 0)
        return;
    if (s_count + count > kCapacity)
        count = kCapacity - s_count;
    for (int i = 0; i < count; ++i)
        s_buffer[s_count + i] = (uint16_t)((int32_t)samples[i] + 0x8000);
    s_count += count;
}
