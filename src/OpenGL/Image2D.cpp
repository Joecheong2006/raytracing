#include "Image2D.h"
#include "glad/glad.h"

namespace gl {
    Image2D::Image2D(i32 width, i32 height, u32 access)
        : m_width(width), m_height(height)
    {
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindImageTexture(0, m_id, 0, GL_FALSE, 0, access, GL_RGBA32F);
    }

    Image2D::Image2D(i32 width, i32 height)
        : Image2D(width, height, GL_READ_WRITE)
    {}

    Image2D::~Image2D() {
        glDeleteTextures(1, &m_id);
    }

    void Image2D::bindTexture(u32 slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    void Image2D::bind(u32 slot, i32 access) const {
        glBindImageTexture(slot, m_id, 0, GL_FALSE, 0, access, GL_RGBA32F);
    }

    void Image2D::bind(u32 slot) const {
        bind(slot, GL_READ_WRITE);
    }

    void Image2D::unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Image2D::resize(i32 width, i32 height) {
        m_width = width;
        m_height = height;

        glDeleteTextures(1, &m_id);

        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindImageTexture(0, m_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    }

}
