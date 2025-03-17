#pragma once
#include "util.h"

class VertexBuffer
{
private:
    u32 m_id;

public:
    VertexBuffer(const void* data, u32 size);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;

};
