#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Texture2D.h"
#include "stb_image.h"
#include "glfw3.h"
#include "imgui/imgui.h"

struct Quad {
    VertexArray vao;
    VertexBuffer vbo;
    IndexBuffer ibo;
    Quad(f32* vertices, u32* indices)
        : vbo(vertices, sizeof(f32) * 8), ibo(indices, 6)
    {
        VertexBufferLayout layout;
        layout.add<float>(2);
        vao.apply_buffer_layout(layout);
    }
};

class Application {
private:
    GLFWwindow* m_window;
    ImGuiIO m_imguiIO;
    static ShaderProgram* currentShader;
    static std::string shaderName;
    static void loadShader(const std::string& name);

    friend void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void mouse_callback(GLFWwindow* window, double xpos, double ypos);

    struct Camera {
        glm::vec3 pos;
        float yaw = 90, pitch = 0, fov = 45;

        glm::vec3 up = glm::vec3(0, 1, 0),
                  right = glm::vec3(1, 0, 0),
                  forward = glm::vec3(0, 0, 1);
    } cam;

    glm::vec2 resolution;
    bool focus = true;
    float frameIndex = 0;
    int bounces = 10;

    void update();
    void render(const Quad& quad);
    void imguiRender();

public:
    Application();
    ~Application();

    void run();

};
