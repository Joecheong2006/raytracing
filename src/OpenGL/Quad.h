#ifndef QUAD_H
#define QUAD_H

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"

struct Quad {
    VertexArray vao;
    VertexBuffer vbo;
    IndexBuffer ibo;
    Quad() = default;
    Quad(f32* vertices, u32* indices)
        : vbo(vertices, sizeof(f32) * 8), ibo(indices, 6)
    {
        VertexBufferLayout layout;
        layout.add<float>(2);
        vao.apply_buffer_layout(layout);
    }
};

#endif
