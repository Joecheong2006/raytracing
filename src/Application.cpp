#include "Application.h"
#include "OpenGL/VertexBufferLayout.h"
#include "glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "glm/gtc/type_ptr.hpp"
#include <fstream>
#include <cstring>
#include <string>

#include "OpenGL/ShaderStorageBuffer.h"

#define SHADER_SOURCE_DIRECTORY "shaders/"
#define DEFAULT_SHADER_NAME "default"

gl::ShaderProgram* Application::currentShader;
std::string Application::shaderName = "ray-tracing";

static f32 vertices[] = {
     1.0,  1.0, 1, 1,
    -1.0,  1.0, 0, 1,
     1.0, -1.0, 1, 0,
    -1.0, -1.0, 0, 0
};

static u32 indices[] = {
    0, 1, 2,
    1, 3, 2,
};

inline bool file_exist(const std::string& path) {
    std::ifstream file(path);
    return file.is_open();
}

inline bool shader_path_exist(const std::string& name) {
    return file_exist(SHADER_SOURCE_DIRECTORY + name + ".vert") && file_exist(SHADER_SOURCE_DIRECTORY + name + ".frag");
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Application::Detail& det = static_cast<Application*>(glfwGetWindowUserPointer(window))->detail;
    det.resolution.x = width;
    det.resolution.y = height;
    glViewport(0, 0, width, height);
    det.screen = std::vector<float>(width * height * 4, 0.0f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && Application::currentShader) {
        if (shader_path_exist(Application::shaderName)) {
            Application::loadShader(Application::shaderName);
        } else {
            Application::loadShader(DEFAULT_SHADER_NAME);
        }
    }

    Application::Detail& det = static_cast<Application*>(glfwGetWindowUserPointer(window))->detail;
    if(!det.focus && key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        det.focus = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        return;
    }

    if(det.focus && key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        det.focus = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    Application::Detail& det = static_cast<Application*>(glfwGetWindowUserPointer(window))->detail;
    det.cam.fov -= yoffset;
    // det.frameIndex = 1;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool first_mouse = true;
    static glm::vec2 last_pos;

    Application::Detail& det = static_cast<Application*>(glfwGetWindowUserPointer(window))->detail;

    if (!det.focus) {
        first_mouse = true;
        return;
    }

    if (first_mouse) {
        last_pos.x = xpos;
        last_pos.y = ypos;
        first_mouse = false;
    }
    det.frameIndex = 1;

    float xoffset = xpos - last_pos.x;
    float yoffset = last_pos.y - ypos; 
    last_pos.x = xpos;
    last_pos.y = ypos;

    float sensitivity = 0.04f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    det.cam.yaw   += xoffset;
    det.cam.pitch += yoffset;

    if(det.cam.pitch > 89.0f)
        det.cam.pitch = 89.0f;
    if(det.cam.pitch < -89.0f)
        det.cam.pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(180 + det.cam.yaw)) * cos(glm::radians(det.cam.pitch));
    direction.y = sin(glm::radians(det.cam.pitch));
    direction.z = sin(glm::radians(det.cam.yaw)) * cos(glm::radians(det.cam.pitch));
    det.cam.forward = glm::normalize(direction);
    det.cam.right = normalize(-glm::cross(det.cam.forward, glm::vec3(0, 1, 0)));
    det.cam.up = glm::cross(-det.cam.right, det.cam.forward);
}

Application::Application() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    detail.resolution = glm::ivec2(1920, 1280);
    m_window = glfwCreateWindow(detail.resolution.x, detail.resolution.y, "glsl test", NULL, NULL);

    glfwMakeContextCurrent(m_window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, key_callback);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSetScrollCallback(m_window, scroll_callback);
    glfwSetCursorPosCallback(m_window, mouse_callback);

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    stbi_set_flip_vertically_on_load(true);

    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_imguiIO = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

Application::~Application() {
    delete currentShader;
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::loadShader(const std::string& name) {
    currentShader->clearShaderError();
    currentShader->release();
    currentShader->create();
    currentShader->attach_shader(GL_VERTEX_SHADER, SHADER_SOURCE_DIRECTORY + name + ".vert");
    currentShader->attach_shader(GL_FRAGMENT_SHADER, SHADER_SOURCE_DIRECTORY + name + ".frag");
    if (!currentShader->getShaderError().empty()) {
        currentShader->release();
        currentShader->create();
        currentShader->attach_shader(GL_VERTEX_SHADER, SHADER_SOURCE_DIRECTORY DEFAULT_SHADER_NAME ".vert");
        currentShader->attach_shader(GL_FRAGMENT_SHADER, SHADER_SOURCE_DIRECTORY DEFAULT_SHADER_NAME ".frag");
    }
    currentShader->link();
    shaderName = name;
}

void Application::update() {
    float speed = 0.01;
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        speed  *= 2;

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
        detail.frameIndex = 1;
        detail.cam.pos += detail.cam.forward * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
        detail.frameIndex = 1;
        detail.cam.pos -= detail.cam.forward * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
        detail.frameIndex = 1;
        detail.cam.pos += detail.cam.right * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
        detail.frameIndex = 1;
        detail.cam.pos -= detail.cam.right * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        detail.frameIndex = 1;
        detail.cam.pos.y += speed;
    }
}

void Application::render() {
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));

    static double start_time = glfwGetTime();

    currentShader->bind();
    currentShader->set_1i("currentFrame", 0);
    currentShader->set_1i("previousFrame", 1);

    currentShader->set_1i("objectSize.SPHERE", 2);

    currentShader->set_2f("resolution", detail.resolution);
    currentShader->set_1f("frameIndex", detail.frameIndex);
    currentShader->set_1f("time", glfwGetTime() - start_time);

    currentShader->set_1f("cam.fov", detail.cam.fov);
    currentShader->set_3f("cam.position", detail.cam.pos);
    currentShader->set_3f("cam.forward", detail.cam.forward);
    currentShader->set_3f("cam.right", detail.cam.right);
    currentShader->set_3f("cam.up", detail.cam.up);

    currentShader->set_1i("bounces", detail.bounces);
    currentShader->set_1i("rayPerPixel", detail.rayPerPixel);

    GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}

void Application::imguiRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static char buf[64] = "";

    ImGui::Begin("status");
    ImGui::InputText("shader", buf, IM_ARRAYSIZE(buf));
    ImGui::SameLine();
    if (ImGui::Button("load")) {
        if (shader_path_exist(buf)) {
            loadShader(buf);
        } else {
            loadShader(DEFAULT_SHADER_NAME);
        }
    }

    if (ImGui::SliderFloat("zoom", &detail.cam.fov, 1, 179)) {
        detail.frameIndex = 1;
    }
    if (ImGui::SliderInt("bounces", &detail.bounces, 1, 100)) {
        detail.frameIndex = 1;
    }
    if (ImGui::SliderInt("rayPerPixel", &detail.rayPerPixel, 1, 10)) {
        detail.frameIndex = 1;
    }

    ImGui::BeginChild("Scene");
    auto& materials = world->getMaterials();
    bool hasChanged = false;
    for (u32 i = 0; i < materials.size(); ++i) {
        ImGui::PushID(i);
        if (ImGui::ColorEdit3("albedo", glm::value_ptr(materials[i].albedo)) ||
            ImGui::SliderFloat("roughness", &materials[i].roughness, 0, 1) ||
            ImGui::ColorEdit3("emissionColor", glm::value_ptr(materials[i].emissionColor)) ||
            ImGui::SliderFloat("emissionStrength", &materials[i].emissionStrength, 0, 500)) {
            detail.frameIndex = 1;
            hasChanged = true;
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    if (hasChanged) {
        world->updateBuffer();
    }

    ImGui::TextColored(ImVec4(1,1,0,1), "message");
    ImGui::BeginChild("Scrolling");
    for (auto& error : currentShader->getShaderError()) {
        ImGui::Text(error.c_str(), "");
    }
    ImGui::EndChild();       

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void loadCornellBox(World* world, float boxLen = 3, float lightLen = 0.7) {
    glm::vec3 pos = {-boxLen * 0.5, -1.4, 2 * boxLen};
 
    world->add<Quad>(
            { {pos.x, pos.y, pos.z}, {boxLen, 0, 0}, {0, 0, -boxLen} },
            { {1, 1, 1}, 1, {0, 0, 0}, 0 }, false
        );

    world->add<Quad>(
            { {pos.x + boxLen, pos.y + boxLen, pos.z}, {-boxLen, 0, 0}, {0, 0, -boxLen} },
            { {1, 1, 1}, 1, {0, 0, 0}, 0 }, false
        );

    world->add<Quad>(
            { {pos.x + boxLen, pos.y, pos.z}, {-boxLen, 0, 0}, {0, boxLen, 0} },
            { {1, 1, 1}, 1, {0, 0, 0}, 0 }, false
        );

    world->add<Quad>(
            { {pos.x, pos.y, pos.z - boxLen}, {boxLen, 0, 0}, {0, boxLen, 0} },
            { {1, 1, 1}, 1, {0, 0, 0}, 0 }, false
        );

    world->add<Quad>(
            { {pos.x, pos.y, pos.z - boxLen}, {0, boxLen, 0}, {0, 0, boxLen} },
            { {0.63, 0.065, 0.05}, 1, {0, 0, 0}, 0 }, false
        );

    world->add<Quad>(
            { {pos.x + boxLen, pos.y, pos.z}, {0, boxLen, 0}, {0, 0, -boxLen} },
            { {0.14, 0.45, 0.091}, 1, {0, 0, 0}, 0 }, false
        );

    world->add<Quad>(
            { {pos.x + boxLen * 0.5 + lightLen * 0.5, pos.y + boxLen - 0.000001, pos.z - lightLen * 0.5 - lightLen}, {-lightLen, 0, 0}, {0, 0, -lightLen} },
            { {0, 0, 0}, 0, {1, 1, 1}, 150}, false
        );
}

void loadBox(World* world, glm::vec2 len, glm::vec3 pos, float degree) {
    glm::mat3 r = glm::rotate(glm::mat4(1), glm::radians(degree), {0, 1, 0});

    glm::vec3 b1Vertex[] = {
        glm::vec3{len.x, 0, len.x} * 0.5f * r,
        glm::vec3{-len.x, 0, len.x} * 0.5f * r,
        glm::vec3{-len.x, 0, -len.x} * 0.5f * r,
        glm::vec3{len.x, 0, -len.x} * 0.5f * r,
        glm::vec3{len.x, len.y * 2.0f, len.x} * 0.5f * r,
        glm::vec3{-len.x, len.y * 2.0f, len.x} * 0.5f * r,
        glm::vec3{-len.x, len.y * 2.0f, -len.x} * 0.5f * r,
        glm::vec3{len.x, len.y * 2.0f, -len.x} * 0.5f * r,
    };

    world->add<Quad>(
            { pos + b1Vertex[0], b1Vertex[4] - b1Vertex[0], b1Vertex[1] - b1Vertex[0]},
            { {1, 1, 1}, 1, {0, 0, 0}, 0}, false
        );

    world->add<Quad>(
            { pos + b1Vertex[1], b1Vertex[5] - b1Vertex[1], b1Vertex[2] - b1Vertex[1]},
            { {1, 1, 1}, 1, {0, 0, 0}, 0}, false
        );

    world->add<Quad>(
            { pos + b1Vertex[2], b1Vertex[6] - b1Vertex[2], b1Vertex[3] - b1Vertex[2] },
            { {1, 1, 1}, 1, {0, 0, 0}, 0}, false
        );

    world->add<Quad>(
            { pos + b1Vertex[3], b1Vertex[7] - b1Vertex[3], b1Vertex[0] - b1Vertex[3] },
            { {1, 1, 1}, 1, {0, 0, 0}, 0}, false
        );

    world->add<Quad>(
            { pos + b1Vertex[4], b1Vertex[3] - b1Vertex[0], b1Vertex[1] - b1Vertex[0]},
            { {1, 1, 1}, 1, {0, 0, 0}, 0}, false
        );
}

void loadScene1(World* world) {
    world->add<Sphere>(
            { 50, {0, -50 - 0.2, 1.2}, 0 },
            { {0.7, 0.7, 0.7}, 1.0, {0, 0, 0}, 0 }, true
        );
    
    world->add<Sphere>(
            { 5, {0, 3, 15}, 1 },
            { {0, 0, 0}, 0.3, {1, 1, 1}, 20 }, false
        );

    world->add<Sphere>(
            { 0.2, {0, 0, 1.2}, 2 },
            { {0.6, 0.3, 0.5}, 0.0, {0, 0, 0}, 0 }, true
        );

    world->add<Sphere>(
            { 0.2, {0.5, 0, 1.2}, 2 },
            { {0.3, 0.8, 0.5}, 0.1, {0, 0, 0}, 0 }, true
        );
}

void loadScene2(World* world) {
    float boxLen = 3;
    float lightLen = 0.7;
    glm::vec3 pos = {-boxLen * 0.5, -1.4, 2 * boxLen};
    glm::vec3 center = pos + glm::vec3{boxLen, 0, -boxLen} * 0.5f;
    loadCornellBox(world, boxLen, lightLen);

    world->add<Sphere>(
            { 0.5, center + glm::vec3{0, 1, 0}, 2 },
            { {1, 1, 1}, 0.1, {0, 0, 0}, 0 }, true
        );
}

void loadScene3(World* world) {
    float boxLen = 4;
    float lightLen = 1.1;
    glm::vec3 pos = {-boxLen * 0.5, -1.4, 2 * boxLen};
    loadCornellBox(world, boxLen, lightLen);

    glm::vec2 b1Len = {1.2, 2.4};
    glm::vec3 b1Pos = {1.2, 0, 1.3};
    glm::vec3 b2Pos = {2.64, 0, 2.5};
    loadBox(world, b1Len, {pos.x + b1Pos.x, pos.y + b1Pos.y, pos.z - b1Pos.z}, 20);
    loadBox(world, {1.15, 1.15}, {pos.x + b2Pos.x, pos.y + b2Pos.y, pos.z - b2Pos.z}, -17);
}

void Application::run() {
    currentShader = new gl::ShaderProgram();
    if (shader_path_exist(shaderName)) {
        loadShader(shaderName);
    } else {
        loadShader(DEFAULT_SHADER_NAME);
    }

    gl::ShaderProgram screenShader;
    screenShader.attach_shader(GL_VERTEX_SHADER, SHADER_SOURCE_DIRECTORY "screen.vert");
    screenShader.attach_shader(GL_FRAGMENT_SHADER, SHADER_SOURCE_DIRECTORY "screen.frag");
    screenShader.link();
    screenShader.bind();

    quad = new gl::Quad(vertices, indices);

    glClear(GL_COLOR_BUFFER_BIT);

    detail.cam.pos = glm::vec3(0);
    detail.cam.forward = glm::vec3(0, 0, 1);

    GLCALL(glClearColor(0.1, 0.1, 0.1, 1));

    detail.screen = std::vector<float>(detail.resolution.y * detail.resolution.x * 4, 0.0f);
    gl::ShaderStorageBuffer screenBuffer(detail.screen.data(), detail.screen.size() * sizeof(float));

    world = new World();

    // loadScene1(world);
    // loadScene2(world);
    loadScene3(world);

    world->updateBuffer();

    while(!glfwWindowShouldClose(m_window))
    {
        static double st;
        st = glfwGetTime();

        quad->vao.bind();
        world->bindBuffer();
        render();
        world->unbindBuffer();
        screenBuffer.unbind();

        quad->vao.bind();
        screenShader.bind();
        screenBuffer.binding(0);
        screenShader.set_2f("resolution", detail.resolution);
        GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

        std::string dur = "render: " + std::to_string((glfwGetTime() - st) * 1000.0) + "ms"
                        + " frameIndex: " + std::to_string(detail.frameIndex);
        glfwSetWindowTitle(m_window, dur.c_str());

        ++detail.frameIndex;

        if (detail.focus) {
            update();
        }

        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }

        imguiRender();

        glfwSwapBuffers(m_window);
        glm::ivec2 tmp = detail.resolution;
        glfwPollEvents();

        if (tmp != detail.resolution) {
            screenBuffer.setBuffer(detail.screen.data(), detail.screen.size() * sizeof(float));
            detail.frameIndex = 1;
        }
    }
}

