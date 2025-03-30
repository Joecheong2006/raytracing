#include "Texture2D.h"
#include "glad/glad.h"
#include "stb_image.h"

namespace gl {
    void Texture2D::initialize(i32 width, i32 height, u32 internal_format, u32 format, u32 type, u32 wrap, u32 min_filter, u32 mag_filter) {
        GLCALL(glGenTextures(1, &m_id));
        GLCALL(glBindTexture(GL_TEXTURE_2D, m_id));

        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));	
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));

        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter));

        GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, NULL));
        m_width = width;
        m_height = height;
    }

    Texture2D::Texture2D(i32 width, i32 height, u32 internal_format, u32 format, u32 type)
        : Texture2D(width, height, internal_format, format, type, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR)
    {}

    Texture2D::Texture2D(i32 width, i32 height, u32 internal_format, u32 format, u32 type, u32 wrap, u32 min_filter, u32 mag_filter) {
        initialize(width, height, internal_format, format, type, wrap, min_filter, mag_filter);
    }

    Texture2D::Texture2D(const std::string& path)
        : Texture2D(path, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR)
    {}

    Texture2D::Texture2D(const std::string& path, u32 wrap, u32 min_filter, u32 mag_filter) {
        unsigned char *data = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 0);
        u32 format = m_bpp == 4 ? GL_RGBA : GL_RGB;
        initialize(m_width, m_height, format, format, GL_UNSIGNED_BYTE, wrap, min_filter, mag_filter);

        GLCALL(glGenerateMipmap(GL_TEXTURE_2D));

        if(data) stbi_image_free(data);
    }

    Texture2D::~Texture2D()
    {
        GLCALL(glDeleteTextures(1, &m_id));
    }

    void Texture2D::storage(i32 width, i32 height, i32 format) {
        GLCALL(glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height));
    }

    void Texture2D::bind(u32 slot) const
    {
        GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
        GLCALL(glBindTexture(GL_TEXTURE_2D, m_id));
    }

    void Texture2D::bindImage(u32 access, u32 format) {
        glBindImageTexture(0, m_id, 0, GL_FALSE, 0, access, format);
    }

    void Texture2D::unbind() const
    {
        GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
    }

}
