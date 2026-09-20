// ============================================================================
// BinaryBuffer.cpp —— CBinaryBuffer 完整实现
// 行为基准: 前端 _19.js class _933 (与固件逐语义同源, 大端序)
// ============================================================================
#include "BinaryBuffer.h"

#include <cstdlib>
#include <cstring>

CBinaryBuffer::~CBinaryBuffer() { Free(); }

void CBinaryBuffer::Free() {
    if (m_owned && m_buffer)
        free(m_buffer);
    m_buffer = nullptr;
    m_length = 0;
    m_position = 0;
    m_capacity = 0;
    m_owned = false;
    m_measuring = false;
}

void CBinaryBuffer::StartBuild() {
    Free();
    m_measuring = true;      // JS _935(): position=0; _1077=true
    m_position = 0;
}

bool CBinaryBuffer::EndBuild() {
    // JS _936(): 按干跑出的尺寸分配, 游标归零, 转入真写
    int need = m_position;
    m_measuring = false;
    m_position = 0;
    m_length = 0;
    if (need <= 0) {
        m_capacity = 0;
        return true;
    }
    uint8_t* p = static_cast<uint8_t*>(malloc(need));
    if (!p)
        return false;
    m_buffer = p;
    m_capacity = need;
    m_owned = true;
    return true;
}

bool CBinaryBuffer::Attach(const void* data, int len) {
    Free();
    m_buffer = static_cast<uint8_t*>(const_cast<void*>(data));
    m_length = len;
    m_capacity = len;
    m_owned = false;
    m_measuring = false;
    m_position = 0;
    return true;
}

bool CBinaryBuffer::EnsureCapacity(int need) {
    if (m_measuring) {
        m_position += need;      // 干跑: 只推进游标 (JS _1077 分支)
        return true;
    }
    if (m_position + need <= m_capacity)
        return true;
    // RE SetLength/Append 的扩容分支: realloc; 失败则截断(返回 false 由调用方处理)
    int newCap = m_capacity ? m_capacity * 2 : 256;
    while (newCap < m_position + need)
        newCap *= 2;
    uint8_t* p = static_cast<uint8_t*>(realloc(m_owned ? m_buffer : nullptr, newCap));
    if (!p)
        return false;
    if (!m_owned && m_buffer && m_position > 0)
        memcpy(p, m_buffer, m_position);
    m_buffer = p;
    m_capacity = newCap;
    m_owned = true;
    return true;
}

bool CBinaryBuffer::AppendU32(uint32_t v) {
    if (!EnsureCapacity(4))
        return false;
    if (!m_measuring) {          // 大端: 高字节在前 (固件 bswap 后写)
        m_buffer[m_position + 0] = static_cast<uint8_t>(v >> 24);
        m_buffer[m_position + 1] = static_cast<uint8_t>(v >> 16);
        m_buffer[m_position + 2] = static_cast<uint8_t>(v >> 8);
        m_buffer[m_position + 3] = static_cast<uint8_t>(v);
    }
    m_position += 4;
    if (m_position > m_length)
        m_length = m_position;
    return true;
}

bool CBinaryBuffer::AppendI32(int32_t v) {
    return AppendU32(static_cast<uint32_t>(v));
}

bool CBinaryBuffer::AppendU16(uint16_t v) {
    if (!EnsureCapacity(2))
        return false;
    if (!m_measuring) {
        m_buffer[m_position + 0] = static_cast<uint8_t>(v >> 8);
        m_buffer[m_position + 1] = static_cast<uint8_t>(v);
    }
    m_position += 2;
    if (m_position > m_length)
        m_length = m_position;
    return true;
}

bool CBinaryBuffer::AppendU8(uint8_t v) {
    if (!EnsureCapacity(1))
        return false;
    if (!m_measuring)
        m_buffer[m_position] = v;
    m_position += 1;
    if (m_position > m_length)
        m_length = m_position;
    return true;
}

bool CBinaryBuffer::AppendString(const char* s) {
    if (!s)
        s = "";
    int n = static_cast<int>(strlen(s));
    if (!AppendI32(n))           // JS _1088: i32 长度前缀
        return false;
    return AppendBuffer(s, n);
}

bool CBinaryBuffer::AppendBuffer(const void* p, int n) {
    if (n <= 0)
        return true;
    if (!EnsureCapacity(n))
        return false;
    if (!m_measuring)
        memcpy(m_buffer + m_position, p, n);
    m_position += n;
    if (m_position > m_length)
        m_length = m_position;
    return true;
}

uint32_t CBinaryBuffer::ReadU32() {
    if (!m_buffer || m_position + 4 > m_length) {
        SLG_ASSERT(0);           // 越界读取: 固件原版同样 assert
        return 0;
    }
    uint32_t v = (static_cast<uint32_t>(m_buffer[m_position]) << 24) |
                 (static_cast<uint32_t>(m_buffer[m_position + 1]) << 16) |
                 (static_cast<uint32_t>(m_buffer[m_position + 2]) << 8) |
                 static_cast<uint32_t>(m_buffer[m_position + 3]);
    m_position += 4;
    return v;
}

int32_t CBinaryBuffer::ReadI32() { return static_cast<int32_t>(ReadU32()); }

uint16_t CBinaryBuffer::ReadU16() {
    if (!m_buffer || m_position + 2 > m_length) {
        SLG_ASSERT(0);
        return 0;
    }
    uint16_t v = static_cast<uint16_t>((m_buffer[m_position] << 8) | m_buffer[m_position + 1]);
    m_position += 2;
    return v;
}

uint8_t CBinaryBuffer::ReadU8() {
    if (!m_buffer || m_position + 1 > m_length) {
        SLG_ASSERT(0);
        return 0;
    }
    return m_buffer[m_position++];
}

bool CBinaryBuffer::ReadString8(char* dst, int cap) {
    if (!dst || cap <= 0) {
        SLG_ASSERT(0);
        return false;
    }
    int len = ReadI32();
    // RE 实锤: length>=n → 错误分支 + assert(0) (原文件行 282)
    if (len < 0 || len >= cap) {
        SLG_ASSERT(0);
        dst[0] = '\0';
        return false;
    }
    if (!m_buffer || m_position + len > m_length) {
        SLG_ASSERT(0);
        return false;
    }
    memcpy(dst, m_buffer + m_position, len);
    m_position += len;
    dst[len] = '\0';
    return true;
}

int CBinaryBuffer::ReadBytes(void* dst, int n) {
    if (!m_buffer || n < 0 || m_position + n > m_length)
        return 0;
    if (dst)
        memcpy(dst, m_buffer + m_position, n);
    m_position += n;
    return n;
}

bool CBinaryBuffer::Seek(int offset, SeekOrigin origin) {
    int target;
    switch (origin) {
        case kSeekSet: target = offset; break;
        case kSeekCur: target = m_position + offset; break;
        case kSeekEnd: target = m_length - offset; break;
        default:
            SLG_ASSERT(0);
            return false;
    }
    // RE: assert(pos <= m_length)
    if (target < 0 || target > m_length) {
        SLG_ASSERT(0);
        return false;
    }
    m_position = target;
    return true;
}

bool CBinaryBuffer::SetLength(int length) {
    // RE: malloc/realloc/free 三分支; m_length < pos 时夹紧游标
    if (length < 0)
        return false;
    if (length == 0) {
        Free();
        return true;
    }
    uint8_t* p = static_cast<uint8_t*>(realloc(m_owned ? m_buffer : nullptr, length));
    if (!p)
        return false;
    if (!m_owned && m_buffer && m_length > 0)
        memcpy(p, m_buffer, m_length < length ? m_length : length);
    m_buffer = p;
    m_owned = true;
    m_capacity = length;
    m_length = length;
    if (m_position > m_length)
        m_position = m_length;   // 夹紧游标
    return true;
}
