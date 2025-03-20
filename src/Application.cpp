#include "Application.h"
#include "OpenGL/VertexBufferLayout.h"
#include "glfw3.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <fstream>
#include <cstring>
#include <string>

#include "World.h"

#include "OpenGL/ShaderStorageBuffer.h"

#define SHADER_SOURCE_DIRECTORY "shaders/"
#define DEFAULT_SHADER_NAME "default"

ShaderProgram* Application::currentShader;
std::string Application::shaderName = "ray-tracing";

static f32 vertices[] = {
     1.0,  1.0,
    -1.0,  1.0,
     1.0, -1.0,
    -1.0, -1.0,
};

static u32 indices[] = {
    0, 1, 2,
    1, 3, 2,
};

Quad::Quad(f32* vertices, u32* indices)
    : vbo(vertices, sizeof(f32) * 8), ibo(indices, 6)
{
    VertexBufferLayout layout;
    layout.add<float>(2);
    vao.apply_buffer_layout(layout);
}

inline bool file_exist(const std::string& path) {
    std::ifstream file(path);
    return file.is_open();
}

inline bool shader_path_exist(const std::string& name) {
    return file_exist(SHADER_SOURCE_DIRECTORY + name + ".vert") && file_exist(SHADER_SOURCE_DIRECTORY + name + ".frag");
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->resolution.x = width;
    app->resolution.y = height;
    glViewport(0, 0, width, height);
    app->screen = std::vector<float>(width * height * 4, 0.0f);
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

    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if(!app->focus && key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        app->focus = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        return;
    }

    if(app->focus && key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        app->focus = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    Application::Camera& cam = static_cast<Application*>(glfwGetWindowUserPointer(window))->cam;
    cam.fov -= yoffset;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool first_mouse = true;
    static glm::vec2 last_pos;

    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    Application::Camera& cam = app->cam;

    if (!app->focus) {
        first_mouse = true;
        return;
    }

    if (first_mouse) {
        last_pos.x = xpos;
        last_pos.y = ypos;
        first_mouse = false;
    }
    app->frameIndex = 1;

    float xoffset = xpos - last_pos.x;
    float yoffset = last_pos.y - ypos; 
    last_pos.x = xpos;
    last_pos.y = ypos;

    float sensitivity = 0.04f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    cam.yaw   += xoffset;
    cam.pitch += yoffset;

    if(cam.pitch > 89.0f)
        cam.pitch = 89.0f;
    if(cam.pitch < -89.0f)
        cam.pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(180 + cam.yaw)) * cos(glm::radians(cam.pitch));
    direction.y = sin(glm::radians(cam.pitch));
    direction.z = sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
    cam.forward = glm::normalize(direction);
    cam.right = normalize(-glm::cross(cam.forward, glm::vec3(0, 1, 0)));
    cam.up = glm::cross(-cam.right, cam.forward);
}

Application::Application() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    resolution = glm::ivec2(1920, 1280);
    m_window = glfwCreateWindow(resolution.x, resolution.y, "glsl test", NULL, NULL);

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
        frameIndex = 1;
        cam.pos += cam.forward * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
        frameIndex = 1;
        cam.pos -= cam.forward * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
        frameIndex = 1;
        cam.pos += cam.right * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
        frameIndex = 1;
        cam.pos -= cam.right * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        frameIndex = 1;
        cam.pos.y += speed;
    }
}

void Application::render() {
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));

    static double start_time = glfwGetTime();

    currentShader->bind();
    currentShader->set_1i("currentFrame", 0);
    currentShader->set_1i("previousFrame", 1);

    currentShader->set_1i("objectSize.SPHERE", 2);

    currentShader->set_2f("resolution", resolution);
    currentShader->set_1f("frameIndex", frameIndex);
    currentShader->set_1f("time", glfwGetTime() - start_time);

    currentShader->set_1f("cam.fov", cam.fov);
    currentShader->set_3f("cam.position", cam.pos);
    currentShader->set_3f("cam.forward", cam.forward);
    currentShader->set_3f("cam.right", cam.right);
    currentShader->set_3f("cam.up", cam.up);

    currentShader->set_1i("bounces", bounces);

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

    if (ImGui::SliderFloat("zoom", &cam.fov, 1, 179)) {
        frameIndex = 1;
    }
    if (ImGui::SliderInt("bounces", &bounces, 1, 100)) {
        frameIndex = 1;
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

void Application::run() {
    currentShader = new ShaderProgram();
    if (shader_path_exist(shaderName)) {
        loadShader(shaderName);
    } else {
        loadShader(DEFAULT_SHADER_NAME);
    }

    Quad quad(vertices, indices);
    ShaderProgram screenShader;
    screenShader.attach_shader(GL_VERTEX_SHADER, SHADER_SOURCE_DIRECTORY "screen.vert");
    screenShader.attach_shader(GL_FRAGMENT_SHADER, SHADER_SOURCE_DIRECTORY "screen.frag");
    screenShader.link();
    screenShader.bind();

    glClear(GL_COLOR_BUFFER_BIT);

    cam.pos = glm::vec3(0);
    cam.forward = glm::vec3(0, 0, 1);

    GLCALL(glClearColor(0.1, 0.1, 0.1, 1));

    screen = std::vector<float>(resolution.y * resolution.x * 4, 0.0f);
    ShaderStorageBuffer screenBuffer(screen.data(), screen.size() * sizeof(float));

    std::vector<Sphere> objects = {
        { 50, {0, -50 - 0.2, 1.2}, 0 },
        { 5, {0, 3, 15}, 1 },
        { 0.2, {0, 0, 1.2}, 2 },
    };

    std::vector<Material> materials = {
        { {1, 1, 1}, 0.5, {0, 0, 0},0 },
        { {0.9, 0.5, 0.1}, 0.3, {0.9, 0.5, 0.2}, 22 },
        { {0.6, 0.3, 0.5}, 0.3, {0, 0, 0}, 0 },
    };

    World world;

    world.add<Sphere>(
            { 50, {0, -50 - 0.2, 1.2}, 0 },
            { {1, 1, 1}, 0.5, {0, 0, 0}, 0 }
        );

    world.add<Sphere>(
            { 5, {0, 3, 15}, 1 },
            { {0.9, 0.5, 0.1}, 0.3, {0.9, 0.5, 0.2}, 22 }
        );

    world.add<Sphere>(
            { 0.2, {0, 0, 1.2}, 2 },
            { {0.6, 0.3, 0.5}, 0.3, {0, 0, 0}, 0 }
        );

    world.updateBuffer();

    while(!glfwWindowShouldClose(m_window))
    {
        static double st;
        st = glfwGetTime();

        quad.vao.bind();
        world.updateBuffer();
        world.bindBuffer();
        render();
        world.unbindBuffer();
        screenBuffer.unbind();

        quad.vao.bind();
        screenBuffer.binding(0);
        screenShader.bind();
        screenShader.set_2f("resolution", resolution);
        GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

        std::string dur = "render: " + std::to_string((glfwGetTime() - st) * 1000.0) + "ms"
                        + " frameIndex: " + std::to_string(frameIndex);
        glfwSetWindowTitle(m_window, dur.c_str());

        ++frameIndex;

        if (focus) {
            update();
        }

        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }

        imguiRender();

        glfwSwapBuffers(m_window);
        glm::ivec2 tmp = resolution;
        glfwPollEvents();

        if (tmp != resolution) {
            screenBuffer.setBuffer(screen.data(), screen.size() * sizeof(float));
            frameIndex = 1;
        }
    }
}

