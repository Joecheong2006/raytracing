#ifndef SPHERE_H
#define SPHERE_H

#include "glm/glm.hpp"
#include "AABB.h"

struct Sphere {
    Sphere(float radius, const glm::vec3 center, int materialIndex = 0, int AABBIndex = 0)
        : radius(radius), center(center), materialIndex(materialIndex), aabbIndex(AABBIndex)
    {}

    static AABB GetAABB(const Sphere& sphere) {
        const glm::vec3 v3R = glm::vec3(sphere.radius + 0.01);
        return AABB(sphere.center - v3R, sphere.center + v3R);
    }

    float radius;
    alignas(16) glm::vec3 center;
    int materialIndex;
    int aabbIndex;
};

#endif
