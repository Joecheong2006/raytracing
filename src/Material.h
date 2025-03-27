#ifndef MATERIAL_H
#define MATERIAL_H

#include "glm/glm.hpp"

struct Material {
    alignas(16) glm::vec3 albedo;
    float roughness;

    alignas(16) glm::vec3 emissionColor;
    float emissionStrength;
};

#endif
