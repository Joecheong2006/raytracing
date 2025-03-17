#pragma once

#include "Renderer.h"
#include "util.h"

class Texture2D
{
private:
    u32 m_id;
    i32 m_width, m_height, m_bpp;

public:
    Texture2D(const std::string& path);
    ~Texture2D();

    void bind(u32 slot = 0) const;
    void unbind() const;

    inline i32 width() { return m_width; }
    inline i32 height() { return m_height; }
    inline i32 bpp() { return m_bpp; }

};
