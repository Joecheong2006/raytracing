#pragma once
#include "../util.h"

namespace gl {
    class VertexBuffer
    {
    private:
        u32 m_id;

    public:
        VertexBuffer() = default;
        VertexBuffer(const void* data, u32 size);
        ~VertexBuffer();

        void bind() const;
        void unbind() const;

    };

}
