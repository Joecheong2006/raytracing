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
    gl::ShaderStorageBuffer aabbBuffer;
    u32 aabbBindingIndex = 10;

    std::vector<Material> materials;
    gl::ShaderStorageBuffer materialBuffer;
    u32 materialBindingIndex = 1;

    std::vector<Sphere> spheres;
    gl::ShaderStorageBuffer sphereBuffer;
    u32 sphereBindingIndex = 2;

    std::vector<Quad> quads;
    gl::ShaderStorageBuffer quadBuffer;
    u32 quadBindingIndex = 3;

public:
    glm::vec3 skyColor = {0.5, 0.7, 1};
    float exposure = 1.0, gamma = 2.2;

    struct Camera {
        glm::vec3 pos;
        float yaw = 90, pitch = 0, fov = 45;

        glm::vec3 up = glm::vec3(0, 1, 0),
            right = glm::vec3(1, 0, 0),
            forward = glm::vec3(0, 0, 1);
        int bounces = 5, rayPerPixel = 2;
    } cam;

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

    void fetchBuffer() {
        aabbBuffer.setBuffer(aabbBoxes.data(), aabbBoxes.size() * sizeof(AABB));
        materialBuffer.setBuffer(materials.data(), materials.size() * sizeof(Material));
        sphereBuffer.setBuffer(spheres.data(), spheres.size() * sizeof(Sphere));
        quadBuffer.setBuffer(quads.data(), quads.size() * sizeof(Quad));
    }

    void updateBuffer() {
        aabbBuffer.updateBuffer(aabbBoxes.data(), aabbBoxes.size() * sizeof(AABB));
        materialBuffer.updateBuffer(materials.data(), materials.size() * sizeof(Material));
        sphereBuffer.updateBuffer(spheres.data(), spheres.size() * sizeof(Sphere));
        quadBuffer.updateBuffer(quads.data(), quads.size() * sizeof(Quad));
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
