#include "IndexBuffer.h"
#include "Renderer.h"


IndexBuffer::IndexBuffer(const u32* data, u32 count)
    : m_count(count)
{
    ASSERT(sizeof(u32) == sizeof(GLuint));
    GLCALL(glGenBuffers(1, &m_id));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*count, data, GL_STATIC_DRAW));

}

IndexBuffer::~IndexBuffer()
{
    GLCALL(glDeleteBuffers(1, &m_id));
}

void IndexBuffer::bind() const
{
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
}

void IndexBuffer::unbind() const
{
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
