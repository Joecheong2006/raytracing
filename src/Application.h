#pragma once
#include "OpenGL/Quad.h"
#include "stb_image.h"
#include "glfw3.h"
#include "imgui/imgui.h"
#include "World.h"

class Application {
private:
    GLFWwindow* m_window;
    ImGuiIO m_imguiIO;

    friend void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void mouse_callback(GLFWwindow* window, double xpos, double ypos);

    gl::Quad* quad;
    struct Detail {
        glm::ivec2 resolution;
        bool focus = true;
        int toneMappingMethodIdx = 0;
        unsigned int frameIndex = 1;
    } detail;

    World* world;

    void update();
    void imguiRender();

public:
    Application();
    ~Application();

    void run();

};
