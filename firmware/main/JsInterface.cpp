// ============================================================================
// JsInterface.cpp —— BLE 命令协议层实现 (25 上行命令分发 + 16 下行响应)
// ----------------------------------------------------------------------------
// 还原来源:
//   * CJsInterface::Drive()    @0x400DCBE8 (strcmp 分发链)
//   * OnBluetoothDataCallback  @0x400DC834 (memcpy 进静态缓冲)
//   * 前端 _27.js class _1170 逐字段对照 (置信度 ★★★★★)
//
// 协议 (两端交叉验证, 大端):
//   上行: [u32 BE 魔数 0x4E8A2F5B][string 命令名][负载]          (JS _8157 发送)
//   下行: [string 命令名][i32 total][i32 pos][i32 chunkLen][data] (无魔数!)
//         单帧 ≤509B; JS _1191 按 total/pos 重组, position>=total 时分发。
//         注意 CurrentOutput / ShowDebugString / FirmwareVersion 等
//         "纯文本"响应负载 = 原始 ASCII, 不带 i32 长度前缀 (JS 直接
//         TextDecoder 解码 _8138 全体 —— 已从 _27.js 49-58/97 行实证)。
//   分块上传 (UpdateFirmwareData / DownloadPickupPlugin / AutoInstrumentAudioData,
//   共用 _613.js _8248 引擎, 两遍构建):
//         负载 = [i32 块序][i32 总块数][i32 片序][i32 片大小][i32 本块片数][数据]
//         外层 4096B 块 × 内层 455B 片; 全局偏移 = 块序×4096 + 片序×455
// ============================================================================

#include "JsInterface.h"
#include "UserConfigure.h"
#include "SilkLegGuitar.h"
#include "PickupPlugin.h"
#include "RecordTemporarily.h"
#include "driver/BluetoothCustomService.h"
#include "driver/BluetoothManager.h"

#include <cstdio>
#include <cstring>

// 堆大小桩 —— 实现在文件底部 (把 ESP 依赖隔离, 便于宿主端单测协议层)
unsigned GetFreeHeapSizeStub();

// ---- 静态成员 ---------------------------------------------------------------
uint8_t CJsInterface::s_rxBuffer[512];
int     CJsInterface::s_rxLength = 0;

// ============================================================================
// 收包
// ============================================================================

// 固件 OnBluetoothDataCallback @0x400DC834: memcpy 进静态缓冲后记长度。
// 骨架增加分片续攒: GATT WRITE 事件可能把一个逻辑包拆成多次送达 (MTU 不足时),
// Drive() 里按 "魔数+命令名长度" 预检, 不完整则保留缓冲等下一片。 TODO: 与
// 固件原始行为核对 (RE 只确认了 memcpy+记长度, 分片处理方式未反出)。
void CJsInterface::OnBluetoothData(const void* data, int len)
{
    if (data == nullptr || len <= 0)
        return;
    if (len > (int)sizeof(s_rxBuffer)) {
        printf("Error CJsInterface::OnBluetoothData overflow %d\n", len);
        s_rxLength = 0;   // 失步, 丢弃整包等待魔数重新同步
        return;
    }
    memcpy(s_rxBuffer, data, len);
    s_rxLength = len;
}

// ============================================================================
// 发包
// ============================================================================

// 单帧指示 → GATT 特征 0x6710 (need_confirm=1)。固件断言 ≤0x1FD=509。
bool CJsInterface::SendIndicateFrame(const void* data, int len)
{
    if (len > slg::kMaxIndicateSize || len < 0) {
        SLG_ASSERT(0);
        return false;
    }
    return CBluetoothCustomService::SendIndicate(data, len);
}

// 通用下行: [string cmd][i32 total][i32 pos][i32 chunkLen][data], 两遍构建。
// JS _1191: pos==0 → 按 total 重建 _8138; 每帧 assert position==pos;
// position>=total → 归零并按命令名分发 _8138。
void CJsInterface::SendResponse(const char* cmd, const void* payload, int total)
{
    if (cmd == nullptr)
        return;
    const int cmdLen       = (int)strlen(cmd);
    const int frameOverhead = 4 + cmdLen + 12;              // string 头 + total/pos/chunkLen
    const int maxChunk     = slg::kMaxIndicateSize - frameOverhead;
    if (maxChunk <= 0 || total < 0 || (total > 0 && payload == nullptr)) {
        SLG_ASSERT(0);
        return;
    }
    const uint8_t* p = (const uint8_t*)payload;

    int pos = 0;
    do {
        int chunk = total - pos;
        if (chunk > maxChunk)
            chunk = maxChunk;

        CBinaryBuffer frame;                                 // 两遍构建 (JS _934<2 循环同源)
        for (int pass = 0; pass < 2; ++pass) {
            if (pass == 0)
                frame.StartBuild();                          // 干跑
            else if (!frame.EndBuild()) {                    // 分配+真写
                SLG_ASSERT(0);
                return;
            }
            frame.AppendString(cmd);
            frame.AppendI32(total);
            frame.AppendI32(pos);
            frame.AppendI32(chunk);
            if (chunk > 0)
                frame.AppendBuffer(p + pos, chunk);
        }
        if (!SendIndicateFrame(frame.Data(), frame.Length()))
            return;                                          // 断连即止
        pos += chunk;
    } while (pos < total);
}

// 纯文本响应: 负载 = 原始 ASCII (无长度前缀)。
// 适用于 ShowDebugString / ShowCpuUsageString / ShowMemroyUsageString /
// FirmwareVersion / CurrentOutput (JS 直接 TextDecoder('ascii') 解全体)。
void CJsInterface::SendResponseString(const char* cmd, const char* text)
{
    int n = (text != nullptr) ? (int)strlen(text) : 0;
    SendResponse(cmd, text, n);
}

// ============================================================================
// 内部小工具
// ============================================================================

namespace {

// 字符串 → EOutput (前端 _8172 发送的 5 个名字, 见 Common.h GetOutputString)
bool ParseOutput(const char* name, EOutput* out)
{
    for (int i = 0; i <= 5; ++i) {
        EOutput o = (EOutput)i;
        if (strcmp(GetOutputString(o), name) == 0) {
            *out = o;
            return true;
        }
    }
    return false;
}

bool ParseModule(const char* name, EModule* out)
{
    for (int i = 1; i <= 3; ++i) {
        EModule m = (EModule)i;
        if (strcmp(GetModuleString(m), name) == 0) {
            *out = m;
            return true;
        }
    }
    return false;
}

// 录音数据下行: 负载 = [i32 BE 采样率][PCM: LE u16, 每样本 = s16 + 0x8000]。
// JS _8142: _1100() 读大端 i32 采样率, 其余字节 new Uint16Array(...) 按
// 原生(小端)端序解释后统一减 0x8000 —— 音频批量数据走小端 memcpy, 与协议
// 整数的大端并存 (Xtensa 原生 LE)。
void SendRecord(const char* cmd, int sampleRate, const uint16_t* biased, int sampleCount)
{
    CBinaryBuffer b;
    for (int pass = 0; pass < 2; ++pass) {
        if (pass == 0)      b.StartBuild();
        else if (!b.EndBuild()) { SLG_ASSERT(0); return; }
        b.AppendI32((int32_t)sampleRate);
        if (sampleCount > 0)
            b.AppendBuffer(biased, sampleCount * 2);         // 原样 memcpy (LE)
    }
    CJsInterface::SendResponse(cmd, b.Data(), b.Length());
}

// ---- 分块上传接收状态 -------------------------------------------------------
// 三条命令共用 _8248 引擎格式, 但数据汇不同 (OTA flash / 插件 / 歌曲)。
struct ChunkHeader {
    int32_t block;          // 块序号 (0..totalBlocks-1)
    int32_t totalBlocks;
    int32_t piece;          // 块内片序号 (0..piecesInBlock-1)
    int32_t pieceSize;      // 本片数据字节数 (=455, 末片可短)
    int32_t piecesInBlock;  // 本块片数
};

bool ReadChunkHeader(CBinaryBuffer& rx, ChunkHeader* h)
{
    h->block         = rx.ReadI32();
    h->totalBlocks   = rx.ReadI32();
    h->piece         = rx.ReadI32();
    h->pieceSize     = rx.ReadI32();
    h->piecesInBlock = rx.ReadI32();
    if (h->totalBlocks <= 0 || h->block < 0 || h->block >= h->totalBlocks)
        return false;
    if (h->piecesInBlock <= 0 || h->piece < 0 || h->piece >= h->piecesInBlock)
        return false;
    if (h->pieceSize < 0 || h->pieceSize > slg::kUploadPieceSize)
        return false;
    return true;
}

// ---- OTA 状态 (UpdateFirmwareData 路径; flash 写入为桩) ----------------------
struct OtaState {
    bool     active   = false;
    int32_t  blocks   = 0;
    int32_t  lastBlock = -1;
    int32_t  lastPiece = -1;
    uint32_t received = 0;
};
OtaState s_ota;

void OtaFinalize(bool success)
{
    s_ota = OtaState{};
    // JS _8154 收 Success → 提示重启; _Failed → alert("固件升级失败")
    CJsInterface::SendResponse(success ? "UpdateFirmwareData_Success" : "UpdateFirmwareData_Failed",
                               nullptr, 0);
}

} // namespace

// ============================================================================
// Drive —— strcmp 分发链 (固件原结构, @0x400DCBE8)
// ============================================================================

bool CJsInterface::Drive()
{
    // 包最小骨架: u32 魔数 + i32 命令名长度 = 8B; 不够则等下一片
    if (s_rxLength < 8)
        return false;

    // 预检命令名长度 (大端 i32 @+4), 避免畸形包进 ReadString8 触发断言
    // (ReadString8 对 len>=cap 断言 —— 原固件行 282 行为; 骨架先挡掉脏数据)
    const int32_t cmdLen = (int32_t)((uint32_t)s_rxBuffer[4] << 24 |
                                     (uint32_t)s_rxBuffer[5] << 16 |
                                     (uint32_t)s_rxBuffer[6] << 8  |
                                     (uint32_t)s_rxBuffer[7]);
    if (cmdLen <= 0 || cmdLen > 60) {
        printf("Error CJsInterface::Drive bad cmd length %d\n", cmdLen);
        s_rxLength = 0;
        return false;
    }
    if (s_rxLength < 8 + cmdLen)
        return false;                    // 分片未齐, 继续攒

    CBinaryBuffer rx;
    rx.Attach(s_rxBuffer, s_rxLength);

    const uint32_t magic = rx.ReadU32();
    if (magic != slg::kPacketMagic) {
        // TODO: 原固件对坏魔数的处理未反出 (RE 只见正常路径), 骨架丢弃重同步
        printf("Error CJsInterface::Drive bad magic 0x%08X\n", magic);
        s_rxLength = 0;
        return false;
    }

    char cmd[64] = {0};
    if (!rx.ReadString8(cmd, (int)sizeof(cmd))) {
        s_rxLength = 0;
        return false;
    }

    bool handled = false;

    // ------------------------------------------------------------------
    // 无负载查询类
    // ------------------------------------------------------------------
    if (strcmp(cmd, "GetDebugString") == 0) {
        char buf[128];
        snprintf(buf, sizeof(buf), "silk leg guita %s up\n", slg::kFirmwareVersion);
        SendResponseString("ShowDebugString", buf);       // TODO: 与原调试串格式核对
        handled = true;
    } else if (strcmp(cmd, "GetCpuUsageString") == 0) {
        // TODO: 原固件输出 CPU 占用率文本; 空闲 task 计数法待实现
        SendResponseString("ShowCpuUsageString", "cpu: n/a\n");
        handled = true;
    } else if (strcmp(cmd, "GetMemoryUsageString") == 0) {
        // 命令名 "Memroy" 为固件原生拼写 (前端 _27.js case 同名依赖, 勿改)
        // TODO: 原串含堆/最小堆/ DMA 分区统计, RE 未见完整格式
        char buf[128];
        snprintf(buf, sizeof(buf), "memory: %u\n", (unsigned)GetFreeHeapSizeStub());
        SendResponseString("ShowMemroyUsageString", buf);
        handled = true;
    } else if (strcmp(cmd, "GetFirmwareVersion") == 0) {
        SendResponseString("FirmwareVersion", slg::kFirmwareVersion);   // JS _8145 比对
        handled = true;
    } else if (strcmp(cmd, "GetUserConfigure") == 0) {
        uint8_t blob[256];
        const int n = CUserConfigure::Serialize(blob, (int)sizeof(blob));
        if (n > 0)
            SendResponse("UserConfigure", blob, n);
        handled = true;
    } else if (strcmp(cmd, "GetPickupPluginBaseAddress") == 0) {
        // 负载 = u32 BE 代码段基址 + u32 BE 数据段基址 (JS _8146/_8147, _1101)
        CBinaryBuffer b;
        for (int pass = 0; pass < 2; ++pass) {
            if (pass == 0)      b.StartBuild();
            else if (!b.EndBuild()) { SLG_ASSERT(0); break; }
            b.AppendU32(CPickupPlugin::GetCodeBase());
            b.AppendU32(CPickupPlugin::GetDataBase());
        }
        SendResponse("PickupPluginBaseAddress", b.Data(), b.Length());
        handled = true;
    } else if (strcmp(cmd, "GetPickupPluginHtml") == 0) {
        // 嵌套格式: SendResponse 负载 = [u32 offset][u32 total][html...]
        // (JS _8149: offset==0 → _8139 重置分配; 攒满 total 渲染 blob iframe)
        int htmlLen = 0;
        const uint8_t* html = CPickupPlugin::GetHtml(&htmlLen);
        if (html != nullptr && htmlLen > 0) {
            CBinaryBuffer b;
            for (int pass = 0; pass < 2; ++pass) {
                if (pass == 0)      b.StartBuild();
                else if (!b.EndBuild()) { SLG_ASSERT(0); break; }
                b.AppendU32(0);                           // offset = 0 (单次全量)
                b.AppendU32((uint32_t)htmlLen);
                b.AppendBuffer(html, htmlLen);
            }
            SendResponse("PickupPluginHtml", b.Data(), b.Length());
        }
        handled = true;
    }
    // ------------------------------------------------------------------
    // 状态查询 (音箱 / MIDI)
    // ------------------------------------------------------------------
    else if (strcmp(cmd, "GetBluetoothDriveSpeakState") == 0) {
        // 负载 = string 状态 [+ 6B MAC + 30B 名 (仅 connected)] (JS _8150)
        CBluetoothManager::SpeakerState st = CBluetoothManager::GetSpeakerState();
        CBinaryBuffer b;
        for (int pass = 0; pass < 2; ++pass) {
            if (pass == 0)      b.StartBuild();
            else if (!b.EndBuild()) { SLG_ASSERT(0); break; }
            b.AppendString(CBluetoothManager::GetSpeakerStateString());
            if (st == CBluetoothManager::SpeakerState::connected) {
                uint8_t mac[6] = {0};                    // TODO: 真实 MAC
                char name[30]  = {0};                    // TODO: 真实名字
                b.AppendBuffer(mac, 6);
                b.AppendBuffer(name, 30);
            }
        }
        SendResponse("BluetoothDriveSpeakState", b.Data(), b.Length());
        handled = true;
    } else if (strcmp(cmd, "GetMidiDeviceState") == 0) {
        // 负载 = u8 enabled + u8 connected + string 名 (JS _8151: _1099×2+_1105)
        CBinaryBuffer b;
        for (int pass = 0; pass < 2; ++pass) {
            if (pass == 0)      b.StartBuild();
            else if (!b.EndBuild()) { SLG_ASSERT(0); break; }
            b.AppendU8(0);                               // enabled  TODO
            b.AppendU8(0);                               // connected TODO
            b.AppendString("");                          // 设备名   TODO
        }
        SendResponse("MidiDeviceState", b.Data(), b.Length());
        handled = true;
    }
    // ------------------------------------------------------------------
    // 录音
    // ------------------------------------------------------------------
    else if (strcmp(cmd, "RecordTemporarily") == 0 || strcmp(cmd, "GetHistoryRecord") == 0) {
        // 负载 = string "input" / "output" (JS _8164/_8165)
        char src[8] = {0};
        rx.ReadString8(src, (int)sizeof(src));
        const bool isInput = (strcmp(src, "input") == 0);
        CRecordTemporarily::Start(isInput ? CRecordTemporarily::Source::input
                                          : CRecordTemporarily::Source::output);
        // 骨架: 立即回发当前缓冲 (空)。TODO: 实机为录满/超时后回发;
        // input→MonoData / output→StereoData 为推断 (通道数 1/2, ◐)。
        SendRecord(isInput ? "RecordTemporarilyMonoData" : "RecordTemporarilyStereoData",
                   slg::kSampleRate,
                   CRecordTemporarily::Data(),
                   CRecordTemporarily::SampleCount());
        handled = true;
    }
    // ------------------------------------------------------------------
    // 配置 / 输出 / 模块切换
    // ------------------------------------------------------------------
    else if (strcmp(cmd, "SetOutput") == 0) {
        char name[32] = {0};
        rx.ReadString8(name, (int)sizeof(name));
        EOutput o = EOutput::none;
        if (ParseOutput(name, &o) && CSilkLegGuitar::SwitchOutput(o)) {
            CUserConfigure::SetString("output", name);   // 缺省 "line_out" (JS _8143)
            CUserConfigure::Commit();
            SendResponseString("CurrentOutput", GetOutputString(o));   // 纯 ASCII
        }
        handled = true;
    } else if (strcmp(cmd, "SetAutoPitch") == 0) {
        CSilkLegGuitar::SetAutoPitch(true);              // TODO: 负载格式未反出
        handled = true;
    } else if (strcmp(cmd, "SetPickup") == 0) {
        CSilkLegGuitar::SetPickup(true);
        handled = true;
    } else if (strcmp(cmd, "SetMidiDevice") == 0) {
        CSilkLegGuitar::SetMidiDevice(true);
        handled = true;
    }
    // ------------------------------------------------------------------
    // 蓝牙音箱 / MIDI 设备
    // ------------------------------------------------------------------
    else if (strcmp(cmd, "Search") == 0) {
        CBluetoothManager::StartSearch();                // 发现→逐台发 Discovered
        handled = true;
    } else if (strcmp(cmd, "BluetoothDriveSpeakerConnect") == 0) {
        // 负载 = 6B MAC 裸字节 + 30B 名字裸字节 (JS _8179: 29 字符 + NUL)
        uint8_t mac[6];
        char name[30];
        rx.ReadBytes(mac, 6);
        rx.ReadBytes(name, 30);
        CBluetoothManager::ConnectSpeaker(mac, name);
        handled = true;
    } else if (strcmp(cmd, "BluetoothDriveSpeakerDisconnect") == 0) {
        CBluetoothManager::DisconnectSpeaker();
        handled = true;
    } else if (strcmp(cmd, "DisconnectMidiDevice") == 0) {
        // TODO: MIDI 设备断开 (BLE MIDI 对端, RE 细节缺)
        handled = true;
    }
    // ------------------------------------------------------------------
    // 电机
    // ------------------------------------------------------------------
    else if (strcmp(cmd, "MountMotor") == 0) {
        // 负载 = string "1"/"2" + string "Up"/"Down" + u32 毫秒 (JS _8187)
        char motor[8] = {0}, dir[8] = {0};
        rx.ReadString8(motor, (int)sizeof(motor));
        rx.ReadString8(dir,   (int)sizeof(dir));
        const uint32_t ms = rx.ReadU32();
        CSilkLegGuitar::MountMotor(motor[0] == '2' ? 2 : 1, dir, ms);
        handled = true;
    }
    // ------------------------------------------------------------------
    // OTA / 插件
    // ------------------------------------------------------------------
    else if (strcmp(cmd, "UpdateFirmwareDataCanceled") == 0) {
        if (s_ota.active) {
            s_ota = OtaState{};
            // TODO: esp_ota_end 中止 + flash 擦除回滚 (骨架为纯状态复位)
        }
        handled = true;
    } else if (strcmp(cmd, "PickupPluginSendToCpp") == 0) {
        // 负载 = 原始字节 ≤459 (JS _8192; 运行时插件↔固件通道)
        const int remain = s_rxLength - rx.Position();
        if (remain > 0)
            CPickupPlugin::SendToCpp(rx.Data() + rx.Position(), remain);
        handled = true;
    }
    // ------------------------------------------------------------------
    // 分块上传 ×3 (共用 _8248 引擎格式)
    // ------------------------------------------------------------------
    else if (strcmp(cmd, "UpdateFirmwareData") == 0 ||
             strcmp(cmd, "DownloadPickupPlugin") == 0 ||
             strcmp(cmd, "AutoInstrumentAudioData") == 0) {
        const bool isOta    = (strcmp(cmd, "UpdateFirmwareData") == 0);
        const bool isPlugin = (strcmp(cmd, "DownloadPickupPlugin") == 0);

        ChunkHeader h;
        if (!ReadChunkHeader(rx, &h)) {
            printf("Error CJsInterface::Drive %s bad chunk header\n", cmd);
            if (isOta) OtaFinalize(false);
            s_rxLength = 0;
            return true;
        }
        const int remain = s_rxLength - rx.Position();
        if (remain != h.pieceSize) {
            // JS 保证 pieceSize==数据长; 不符说明失步
            printf("Error CJsInterface::Drive %s piece size %d != %d\n",
                   cmd, h.pieceSize, remain);
            if (isOta) OtaFinalize(false);
            s_rxLength = 0;
            return true;
        }
        const uint8_t* data = rx.Data() + rx.Position();
        // 绝对偏移 = 块×4096 + 片×455 (与 JS _8266 = _8262 + _8265×_8257 同式)
        const uint32_t offset = (uint32_t)h.block * slg::kUploadBlockSize +
                                (uint32_t)h.piece * slg::kUploadPieceSize;

        if (isOta) {
            if (!s_ota.active) {
                s_ota.active = true;
                s_ota.blocks = h.totalBlocks;
                s_ota.received = 0;
                // TODO: esp_ota_begin(总长 = totalBlocks×4096), 上限 2MB (RE)
            }
            s_ota.received += (uint32_t)h.pieceSize;
            s_ota.lastBlock = h.block;
            s_ota.lastPiece = h.piece;
            // TODO: esp_ota_write(offset, data, pieceSize)
        } else if (isPlugin) {
            CPickupPlugin::OnDownloadData(offset, data, h.pieceSize);
        } else {
            // AutoInstrumentAudioData → 歌曲数据写入 (歌曲格式见逆向报告 4.1)
            // TODO: flash 歌曲区写入 (4096 头区 + 音频区 @28672)
        }

        // 末块末片 → 收尾
        if (h.block == h.totalBlocks - 1 && h.piece == h.piecesInBlock - 1) {
            if (isOta) {
                // TODO: esp_ota_end + set_boot_partition (原固件无签名校验!)
                OtaFinalize(true);
            } else if (isPlugin) {
                CPickupPlugin::FinalizeDownload();
            }
        }
        handled = true;
    }

    if (!handled) {
        // 原固件日志 (字符串区原文): "Error CJsInterface::Drive received not ..."
        printf("Error CJsInterface::Drive received not %s\n", cmd);
    }

    s_rxLength = 0;      // 包已消费
    return handled;
}

// ============================================================================
// 堆大小桩 (把 ESP 依赖隔离在文件底部, 便于宿主端单测 BinaryBuffer/协议层)
// ============================================================================
#if defined(__has_include)
#  if __has_include(<esp_system.h>)
#    include <esp_system.h>
unsigned GetFreeHeapSizeStub() { return (unsigned)esp_get_free_heap_size(); }
#  else
unsigned GetFreeHeapSizeStub() { return 0; }
#  endif
#else
unsigned GetFreeHeapSizeStub() { return 0; }
#endif
