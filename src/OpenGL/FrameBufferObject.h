#pragma once

#include "Texture2D.h"

namespace gl {
    class FrameBufferObject
    {
    private:
        u32 m_id;

    public:
        FrameBufferObject(const Texture2D& tex, u32 slot = 0);
        ~FrameBufferObject();

        void bind() const;
        void bindTexture(const Texture2D& tex, u32 slot = 0);
        void unbind() const;

    };

}
