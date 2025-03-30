#ifndef GLQUAD_H
#define GLQUAD_H

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"

namespace gl {
    struct Quad {
        VertexArray vao;
        VertexBuffer vbo;
        IndexBuffer ibo;
        Quad() = default;
        Quad(f32* vertices, u32* indices)
            : vbo(vertices, sizeof(f32) * 8 * 2), ibo(indices, 6)
        {
            VertexBufferLayout layout;
            layout.add<float>(2);
            layout.add<float>(2);
            vao.apply_buffer_layout(layout);
        }
    };

}

#endif
