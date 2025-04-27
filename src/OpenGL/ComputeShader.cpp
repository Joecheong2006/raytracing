#include "ComputeShader.h"
#include "VertexBufferLayout.h"

namespace gl {
    ComputeShader::ComputeShader(const std::string& path, const glm::vec3& groups)
        : ShaderProgram(), groups(groups)
    {
        attach_shader(GL_COMPUTE_SHADER, path);
        link();
    }

    void ComputeShader::use() const {
        glDispatchCompute(groups.x, groups.y, groups.z);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void ComputeShader::updateGroups(const glm::vec3& groups) {
        this->groups = groups;
    }

}

