#pragma once

#include "Renderer.h"

class FrameBufferObject;

class Texture2D
{
    friend FrameBufferObject;
private:
    u32 m_id;
    i32 m_width, m_height, m_bpp;

    void initialize(i32 width, i32 height, u32 internal_format, u32 format, u32 type, u32 wrap, u32 min_filter, u32 mag_filter);

public:
    Texture2D() = default;
    Texture2D(i32 width, i32 height, u32 internal_format, u32 format, u32 type);
    Texture2D(i32 width, i32 height, u32 internal_format, u32 format, u32 type, u32 wrap, u32 min_filter, u32 mag_filter);
    Texture2D(const std::string& path);
    Texture2D(const std::string& path, u32 wrap, u32 min_filter, u32 mag_filter);
    ~Texture2D();

    void storage(i32 width, i32 height, i32 format);

    void bind(u32 slot = 0) const;
    void bindImage(u32 access, u32 format);
    void unbind() const;

    inline i32 width() { return m_width; }
    inline i32 height() { return m_height; }
    inline i32 bpp() { return m_bpp; }

};
