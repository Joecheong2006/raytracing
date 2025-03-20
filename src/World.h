#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include "util.h"
#include "AABB.h"
#include "Material.h"
#include "Sphere.h"
#include "OpenGL/ShaderStorageBuffer.h"

class World {
private:
    u32 objectCount = 0;
    u32 objectAABBCount = 0;

    std::vector<AABB> aabbBoxes;
    ShaderStorageBuffer aabbBuffer;
    u32 aabbBindingIndex = 10;

    std::vector<Material> materials;
    ShaderStorageBuffer materialBuffer;
    u32 materialBindingIndex = 1;

    std::vector<Sphere> spheres;
    ShaderStorageBuffer sphereBuffer;
    u32 sphereBindingIndex = 2;

public:
    World()
        : aabbBuffer(nullptr, 0)
        , materialBuffer(nullptr, 0)
        , sphereBuffer(nullptr, 0)
    {}

    template <typename T>
    void add(const T&, const Material&, bool aabb = false) {
        (void)aabb;
    }

    void updateBuffer() {
        aabbBuffer.setBuffer(aabbBoxes.data(), aabbBoxes.size() * sizeof(AABB));
        materialBuffer.setBuffer(materials.data(), materials.size() * sizeof(Material));
        sphereBuffer.setBuffer(spheres.data(), spheres.size() * sizeof(Sphere));
    }

    void bindBuffer() {
        aabbBuffer.binding(aabbBindingIndex);
        materialBuffer.binding(materialBindingIndex);
        sphereBuffer.binding(sphereBindingIndex);
    }

    void unbindBuffer() {
        aabbBuffer.bind();
        materialBuffer.unbind();
        sphereBuffer.unbind();
    }

};

template <>
inline void World::add<Sphere>(const Sphere& sphere, const Material& mat, bool aabb) {
    Sphere cpy = sphere;
    cpy.materialIndex = objectCount++;
    materials.push_back(mat);
    spheres.push_back(cpy);

    if (aabb) {
        cpy.AABBIndex = objectAABBCount++;
        aabbBoxes.push_back(Sphere::GetAABB(sphere));
    }
}

#endif
