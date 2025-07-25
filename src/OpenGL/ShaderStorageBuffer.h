#pragma once
#include "../util.h"

namespace gl {
    class ShaderStorageBuffer
    {
    private:
        u32 m_id;

    public:
        ShaderStorageBuffer(const void* data, u32 size);
        ~ShaderStorageBuffer();

        void setBuffer(const void* data, u32 size);
        void updateBuffer(const void* data, u32 size);
        void* getBuffer() const;
        void bind() const;
        void binding(int point = 0) const;
        void unbind() const;

    };

}
