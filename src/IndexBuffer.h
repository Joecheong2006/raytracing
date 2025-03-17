#pragma once
#include "util.h"

class IndexBuffer
{
private:
    u32 m_id;
    u32 m_count;

public:
    IndexBuffer(const u32* data, u32 count);
    ~IndexBuffer();

    void bind() const;
    void unbind() const;

    inline u32 count() const { return m_count; }
};
