#pragma once
#include "../util.h"

namespace gl {
    class Image2D {
    private:
        u32 m_id;
        i32 m_width, m_height;

    public:
        Image2D(i32 width, i32 height, u32 access);
        Image2D(i32 width, i32 height);
        ~Image2D();

        void bindTexture(u32 slot = 0) const;
        void bind(u32 slot, i32 access) const;
        void bind(u32 slot) const;
        void unbind() const;

        void resize(i32 width, i32 height);

    };
}
