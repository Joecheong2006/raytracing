#ifndef AABB_H
#define AABB_H

#include "glm/glm.hpp"

struct AABB {
    alignas(16) glm::vec2 IX, IY, IZ;
    AABB(const glm::vec2& intervalX, const glm::vec2& intervalY, const glm::vec2& intervalZ)
        : IX(intervalX), IY(intervalY), IZ(intervalZ)
    {}

    AABB(const glm::vec3& p1, const glm::vec3& p2)
        : IX((p1.x <= p2.x) ? glm::vec2{p1.x, p2.x} : glm::vec2{p2.x, p1.x}),
          IY((p1.y <= p2.y) ? glm::vec2{p1.y, p2.y} : glm::vec2{p2.y, p1.y}),
          IZ((p1.z <= p2.z) ? glm::vec2{p1.z, p2.z} : glm::vec2{p2.z, p1.z})
    {}
};

#endif
