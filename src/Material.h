#ifndef MATERIAL_H
#define MATERIAL_H

#include "glm/glm.hpp"

struct Material {
    alignas(16) glm::vec3 albedo = glm::vec3(1);
    alignas(16) glm::vec3 emissionColor = glm::vec3(0.0);
    float emissionStrength = 0.0;

    float subsurface = 0.0;
    float roughness = 0.0;
    float metallic = 0.0;
    float specular = 0.5;
    float specularTint = 0.0;
};

#endif
