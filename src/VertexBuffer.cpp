#include "VertexBuffer.h"
#include "Renderer.h"

VertexBuffer::VertexBuffer(const void* data, u32 size)
{
    GLCALL(glGenBuffers(1, &m_id));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_id));
    GLCALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
    GLCALL(glDeleteBuffers(1, &m_id));
}

void VertexBuffer::bind() const
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_id));
}

void VertexBuffer::unbind() const
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
