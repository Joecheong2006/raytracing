#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include "util.h"
#include "AABB.h"
#include "Material.h"
#include "Sphere.h"
#include "OpenGL/ShaderStorageBuffer.h"
#include "Quad.h"

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

    std::vector<Quad> quads;
    ShaderStorageBuffer quadBuffer;
    u32 quadBindingIndex = 3;

public:
    World()
        : aabbBuffer(nullptr, 0)
        , materialBuffer(nullptr, 0)
        , sphereBuffer(nullptr, 0)
        , quadBuffer(nullptr, 0)
    {}

    std::vector<Material>& getMaterials() { return materials; }

    template <typename T>
    void add(const T&, const Material&, bool aabb = false) {
        (void)aabb;
    }

    void updateBuffer() {
        aabbBuffer.setBuffer(aabbBoxes.data(), aabbBoxes.size() * sizeof(AABB));
        materialBuffer.setBuffer(materials.data(), materials.size() * sizeof(Material));
        sphereBuffer.setBuffer(spheres.data(), spheres.size() * sizeof(Sphere));
        quadBuffer.setBuffer(quads.data(), quads.size() * sizeof(Quad));
    }

    void bindBuffer() {
        aabbBuffer.binding(aabbBindingIndex);
        materialBuffer.binding(materialBindingIndex);
        sphereBuffer.binding(sphereBindingIndex);
        quadBuffer.binding(quadBindingIndex);
    }

    void unbindBuffer() {
        aabbBuffer.bind();
        materialBuffer.unbind();
        sphereBuffer.unbind();
        quadBuffer.unbind();
    }

};

template <>
inline void World::add<Sphere>(const Sphere& sphere, const Material& mat, bool aabb) {
    materials.push_back(mat);

    Sphere cpy = sphere;
    cpy.materialIndex = objectCount++;

    if (aabb) {
        cpy.aabbIndex = objectAABBCount++;
        aabbBoxes.push_back(Sphere::GetAABB(sphere));
    }
    else {
        cpy.aabbIndex = -1;
    }
    spheres.push_back(cpy);
}

template <>
inline void World::add<Quad>(const Quad& quad, const Material& mat, bool aabb) {
    materials.push_back(mat);

    Quad cpy = quad;
    cpy.materialIndex = objectCount++;

    if (aabb) {
        cpy.aabbIndex = objectAABBCount++;
        aabbBoxes.push_back(Quad::GetAABB(quad));
    }
    else {
        cpy.aabbIndex = -1;
    }
    quads.push_back(cpy);
}

#endif
