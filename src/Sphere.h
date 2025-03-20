#ifndef SPHERE_H
#define SPHERE_H

#include "glm/glm.hpp"

struct Sphere {
    float radius;
    alignas(16) glm::vec3 center;
    int materialIndex;
};

#endif
