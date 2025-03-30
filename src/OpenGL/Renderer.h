#pragma once

#ifdef __linux__
#define ASSERT(x) if(!(x)) __builtin_trap()
#else
#define ASSERT(x) if(!(x)) __debugbreak()
#endif
#define GLCALL(x) gl::GLClearError();\
    x;\
    ASSERT(gl::GLLogCall(#x, __FILE__, __LINE__))\

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "ShaderProgram.h"

namespace gl {
    void GLClearError();
    bool GLLogCall(const char* function, const char* file, int line);

    class Renderer
    {
    public:
        void draw(const VertexArray& vao, const IndexBuffer& ibo, const ShaderProgram& shader) const;
        void clear();
    };

}
