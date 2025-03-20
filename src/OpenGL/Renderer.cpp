#include "Renderer.h"

void GLClearError()
{
    while(glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while(GLenum error = glGetError())
    {
        printf("[%d:%s %d %s]\n", error, file, line, function);
        return false;
    }
    return true;
}

void Renderer::draw(const VertexArray& vao, const IndexBuffer& ibo, const ShaderProgram& shader) const
{
    shader.bind();
    vao.bind();
    GLCALL(glDrawElements(GL_TRIANGLES, ibo.count(), GL_UNSIGNED_INT, NULL));
}

void Renderer::clear()
{
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));
}

