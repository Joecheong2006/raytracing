#include "ShaderStorageBuffer.h"
#include "glad/glad.h"
#include "Renderer.h"

ShaderStorageBuffer::ShaderStorageBuffer(const void* data, u32 size) {
    GLCALL(glGenBuffers(1, &m_id));
    setBuffer(data, size);
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
    GLCALL(glDeleteBuffers(0, &m_id));
}

void ShaderStorageBuffer::setBuffer(const void* data, u32 size) {
    bind();
    GLCALL(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW));
}

void* ShaderStorageBuffer::getBuffer() const {
    GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id));
    void* mappedData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    GLCALL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
    return mappedData;
}

void ShaderStorageBuffer::bind() const {
    GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id));
    
}

void ShaderStorageBuffer::binding(int point) const {
    GLCALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, point, m_id));
}

void ShaderStorageBuffer::unbind() const {
    GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}
