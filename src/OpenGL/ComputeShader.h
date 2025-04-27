#pragma once
#include "ShaderProgram.h"

namespace gl {
    class ComputeShader : public ShaderProgram {
    private:
        glm::vec3 groups;

    public:
        ComputeShader(const std::string& path, const glm::vec3& groups);
        ~ComputeShader() = default;

        void use() const;
        void updateGroups(const glm::vec3& groups);

    };
    
}
