#pragma once

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include <iostream>


#ifdef __linux__
#define ASSERT(x) if(!(x)) __builtin_trap()
#else
#define ASSERT(x) if(!(x)) __debugbreak()
#endif
#define GLCALL(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))\

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "ShaderProgram.h"

class Renderer
{
public:
    void draw(const VertexArray& vao, const IndexBuffer& ibo, const ShaderProgram& shader) const;
    void clear();
};

