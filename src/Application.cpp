#include "Application.h"
#include "glfw3.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <fstream>
#include <cstring>
#include <string>

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
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
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

    resolution = glm::vec2(1920, 1280);
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
        cam.pos += cam.forward * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.pos -= cam.forward * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.pos += cam.right * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.pos -= cam.right * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cam.pos.y += speed;
    }
}

void Application::render(const Quad& quad) {
    static float st;
    st = glfwGetTime();

    ++frameIndex;
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));

    static double start_time = glfwGetTime();
    
    currentShader->bind();
    quad.vao.bind();

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
    std::string dur = "render: " + std::to_string((glfwGetTime() - st) * 1000) + "ms";
    glfwSetWindowTitle(m_window, dur.c_str());
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

    ImGui::SliderFloat("zoom", &cam.fov, 1, 179);
    ImGui::SliderInt("bounces", &bounces, 1, 30);

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
    Quad quad(vertices, indices);

    currentShader = new ShaderProgram();
    if (shader_path_exist(shaderName)) {
        loadShader(shaderName);
    } else {
        loadShader(DEFAULT_SHADER_NAME);
    }

    cam.pos = glm::vec3(0);
    cam.forward = glm::vec3(0, 0, 1);

    GLCALL(glClearColor(0.1, 0.1, 0.1, 1));

    while(!glfwWindowShouldClose(m_window))
    {
        render(quad);

        if (focus) {
            update();
        }

        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }

        imguiRender();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

