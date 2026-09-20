// ============================================================================
// BinaryBuffer.h —— CBinaryBuffer
// ----------------------------------------------------------------------------
// 还原来源: 固件 6 个方法逐条反汇编 (reversed/reconstructed/main/BinaryBuffer.cpp)
//           + 前端 _19.js class _933 逐语义对照 (两遍构建机制由此确证)
// 内存布局(固件): +0 flag(干跑) / +4 buffer / +8 length / +0xC file_position
// 置信度: ★★★★★ (协议所有收发都经过它, 行为被 JS 端完全印证)
// ============================================================================
#pragma once

#include "Common.h"

class CBinaryBuffer {
public:
    CBinaryBuffer() = default;
    ~CBinaryBuffer();

    // ---- 构建协议: 两遍构建 (JS _935()/_936() 同源) -----------------------
    // 第一遍: StartBuild() 后所有 Append* 只推进游标(干跑);
    // 第二遍: EndBuild() 按干跑出的尺寸分配缓冲, 游标归零后重写一遍真数据
    void StartBuild();
    bool EndBuild();          // 分配失败返回 false

    // ---- 附加外部数据 (JS _1094; 固件 OnBluetoothDataCallback memcpy 后包装) ----
    bool Attach(const void* data, int len);   // 非拥有: 只包指针

    // ---- 追加 (大端!) ------------------------------------------------------
    bool AppendU32(uint32_t v);               // 4B 大端 (RE: bswap 后写, Xtensa 指令实锤)
    bool AppendI32(int32_t v);
    bool AppendU16(uint16_t v);
    bool AppendU8(uint8_t v);
    bool AppendString(const char* s);         // i32 长度前缀 + ASCII 字节 (JS _1088)
    bool AppendBuffer(const void* p, int n);  // 原始字节 (JS _1092/_1089)

    // ---- 读取 (大端!) ------------------------------------------------------
    uint32_t ReadU32();                       // RE: ReadHeader 即读 ≤4B 再 bswap
    int32_t  ReadI32();
    uint16_t ReadU16();
    uint8_t  ReadU8();
    // RE 原型: ReadString8(dst, n); length>=n → 错误 + assert(0) (原文件行 282)
    bool     ReadString8(char* dst, int cap);
    int      ReadBytes(void* dst, int n);     // 拷出 n 字节 (JS _1106/_1108)

    // ---- 游标/尺寸 (RE: Seek 支持 0=SET,1=CUR,2=END; assert pos<=m_length) ----
    enum SeekOrigin { kSeekSet = 0, kSeekCur = 1, kSeekEnd = 2 };
    bool Seek(int offset, SeekOrigin origin);
    bool SetLength(int length);               // RE: malloc/realloc/free 三分支
    int  Length()  const { return m_length; }
    int  Position() const { return m_position; }
    void SetPosition(int p) { m_position = p; }
    const uint8_t* Data() const { return m_buffer; }
    bool IsValid() const { return m_buffer != nullptr; }

private:
    bool   EnsureCapacity(int need);
    void   Free();

    bool     m_measuring = false;   // 对应固件 +0 flag (JS _1077)
    uint8_t* m_buffer    = nullptr;  // +4
    int      m_length    = 0;        // +8  (已写/有效字节数)
    int      m_position  = 0;        // +0xC (游标)
    int      m_capacity  = 0;        // 已分配容量 (骨架实现细节)
    bool     m_owned     = false;    // Attach 的数据不拥有
};
