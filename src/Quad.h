#ifndef QUAD_H
#define QUAD_H

#include "glm/glm.hpp"
#include "AABB.h"

struct Quad {
    Quad(const glm::vec3& q, const glm::vec3& u, const glm::vec3& v)
        : q(q), u(u), v(v)
    {}

    static AABB GetAABB(const Quad& quad) {
        return AABB(quad.q, quad.q + quad.u + quad.v);
    }

    alignas(16) glm::vec3 q, u, v;
    int materialIndex;
    int aabbIndex;
};

#endif
