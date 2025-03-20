#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include "util.h"
#include "Material.h"
#include "Sphere.h"
#include "OpenGL/ShaderStorageBuffer.h"

class World {
private:
    u32 objectCount = 0;

    std::vector<Material> materials;
    ShaderStorageBuffer materialBuffer;
    u32 materialBindingIndex = 1;

    std::vector<Sphere> spheres;
    ShaderStorageBuffer sphereBuffer;
    u32 sphereBindingIndex = 2;

public:
    World()
        : materialBuffer(nullptr, 0)
        , sphereBuffer(nullptr, 0)
    {}

    template <typename T>
    void add(const T&, const Material&) {
    }

    void updateBuffer() {
        materialBuffer.bind();
        materialBuffer.setBuffer(materials.data(), materials.size() * sizeof(Material));

        sphereBuffer.bind();
        sphereBuffer.setBuffer(spheres.data(), spheres.size() * sizeof(Sphere));
    }

    void bindBuffer() {
        materialBuffer.binding(materialBindingIndex);
        sphereBuffer.binding(sphereBindingIndex);
    }

    void unbindBuffer() {
        materialBuffer.binding(0);
        sphereBuffer.binding(0);
    }

};

template <>
inline void World::add<Sphere>(const Sphere& sphere, const Material& mat) {
    Sphere cpy = sphere;
    cpy.materialIndex = objectCount++;
    spheres.push_back(cpy);
    materials.push_back(mat);
}

#endif
