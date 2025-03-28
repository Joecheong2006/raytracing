#pragma once
#include "OpenGL/glQuad.h"
#include "OpenGL/ShaderProgram.h"
#include "stb_image.h"
#include "glfw3.h"
#include "imgui/imgui.h"
#include "World.h"

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

    glQuad* quad;
    struct Detail {
        struct Camera {
            glm::vec3 pos;
            float yaw = 90, pitch = 0, fov = 45;

            glm::vec3 up = glm::vec3(0, 1, 0),
                right = glm::vec3(1, 0, 0),
                forward = glm::vec3(0, 0, 1);
        } cam;

        glm::ivec2 resolution;
        bool focus = true;
        float frameIndex = 1;
        int bounces = 5, rayPerPixel = 2;
        std::vector<float> screen;
    } detail;

    World* world;

    void update();
    void render();
    void imguiRender();

public:
    Application();
    ~Application();

    void run();

};
