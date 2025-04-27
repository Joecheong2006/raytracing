#include "FrameBufferObject.h"
#include <iostream>
#include "glad/glad.h"
#include "Renderer.h"

namespace gl {
    FrameBufferObject::FrameBufferObject(const Texture2D& tex, u32 slot) {
        GLCALL(glGenFramebuffers(1, &m_id));

        bind();
        bindTexture(tex, slot);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
    }

    FrameBufferObject::~FrameBufferObject() {
        glDeleteFramebuffers(1, &m_id);
    }

    void FrameBufferObject::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    }

    void FrameBufferObject::bindTexture(const Texture2D& tex, u32 slot) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.m_id, slot); 
    }

    void FrameBufferObject::unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}
