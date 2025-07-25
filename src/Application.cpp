#include "Application.h"
#include "OpenGL/VertexBufferLayout.h"
#include "glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "glm/gtc/type_ptr.hpp"
#include <cstring>
#include <string>
#include <unordered_map>
#include <sstream>

#include "OpenGL/ComputeShader.h"
#include "OpenGL/Image2D.h"

#define SHADER_SOURCE_DIRECTORY "shaders/"

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Application::Detail& det = static_cast<Application*>(glfwGetWindowUserPointer(window))->detail;
    det.resolution.x = width;
    det.resolution.y = height;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    Application::Detail& det = static_cast<Application*>(glfwGetWindowUserPointer(window))->detail;
    if(!det.focus && key == GLFW_KEY_F && action == GLFW_PRESS) {
        det.focus = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        return;
    }

    if(det.focus && key == GLFW_KEY_F && action == GLFW_PRESS) {
        det.focus = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->world->cam.fov -= yoffset;
    app->detail.frameIndex = 1;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool first_mouse = true;
    static glm::vec2 last_pos;

    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (!app->detail.focus) {
        first_mouse = true;
        return;
    }

    if (first_mouse) {
        last_pos.x = xpos;
        last_pos.y = ypos;
        first_mouse = false;
    }
    app->detail.frameIndex = 1;

    float xoffset = xpos - last_pos.x;
    float yoffset = last_pos.y - ypos; 
    last_pos.x = xpos;
    last_pos.y = ypos;

    float sensitivity = 0.04f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    app->world->cam.yaw   += xoffset;
    app->world->cam.pitch += yoffset;

    if(app->world->cam.pitch > 89.0f)
        app->world->cam.pitch = 89.0f;
    if(app->world->cam.pitch < -89.0f)
        app->world->cam.pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(180 + app->world->cam.yaw)) * cos(glm::radians(app->world->cam.pitch));
    direction.y = sin(glm::radians(app->world->cam.pitch));
    direction.z = sin(glm::radians(app->world->cam.yaw)) * cos(glm::radians(app->world->cam.pitch));
    app->world->cam.forward = glm::normalize(direction);
    app->world->cam.right = normalize(-glm::cross(app->world->cam.forward, glm::vec3(0, 1, 0)));
    app->world->cam.up = glm::cross(-app->world->cam.right, app->world->cam.forward);
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
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::update() {
    float speed = 0.01;
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        speed  *= 2;

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
        detail.frameIndex = 1;
        world->cam.pos += world->cam.forward * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
        detail.frameIndex = 1;
        world->cam.pos -= world->cam.forward * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
        detail.frameIndex = 1;
        world->cam.pos += world->cam.right * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
        detail.frameIndex = 1;
        world->cam.pos -= world->cam.right * speed;
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        detail.frameIndex = 1;
        world->cam.pos.y += speed;
    }
}

void Application::imguiRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("status");

    if (ImGui::SliderFloat("zoom", &world->cam.fov, 1, 179) ||
        ImGui::SliderInt("bounces", &world->cam.bounces, 1, 100) ||
        ImGui::SliderInt("rayPerPixel", &world->cam.rayPerPixel, 1, 10) ||
        ImGui::SliderFloat("gamma", &world->gamma, 1, 10) ||
        ImGui::SliderFloat("exposure", &world->exposure, 0, 10) ||
        ImGui::ColorEdit3("skyColor", glm::value_ptr(world->skyColor))
        ) {
        detail.frameIndex = 1;
    }

    const char* items[] = { "None", "Agx", "Reinhard ", "Filmic", "Lottes", "PBRneutral", "Uncharted", "Zcam", "Aces" };
    static const char* currentItem = items[0];
    static std::unordered_map<std::string, int> itemsIdxMap = [items]() {
        std::unordered_map<std::string, int> ret;
        for (int i = 0; i < (int)(sizeof(items) / sizeof(const char*)); ++i) {
            ret[items[i]] = i;
        }
        return ret;
    }();

    ImGuiStyle& style = ImGui::GetStyle();
    float w = ImGui::CalcItemWidth();
    float spacing = style.ItemInnerSpacing.x;
    float button_sz = ImGui::GetFrameHeight();
    ImGui::PushItemWidth(w - spacing * 2.0f - button_sz * 2.0f);
    if (ImGui::BeginCombo("##combo", currentItem)) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (currentItem == items[n]);
            if (ImGui::Selectable(items[n], is_selected)) {
                currentItem = items[n];
                detail.frameIndex = 1;
                detail.toneMappingMethodIdx = itemsIdxMap[currentItem];
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    ImGui::BeginTabBar("mats");
    auto& materials = world->getMaterials();
    bool hasChanged = false;
    for (u32 i = 0; i < materials.size(); ++i) {
        ImGui::NewLine();
        ImGui::PushID(i);
        if (ImGui::ColorEdit3("emissionColor", glm::value_ptr(materials[i].emissionColor)) ||
            ImGui::SliderFloat("emissionStrength", &materials[i].emissionStrength, 0, 500) ||
            ImGui::ColorEdit3("albedo", glm::value_ptr(materials[i].albedo)) ||
            ImGui::SliderFloat("subsurface", &materials[i].subsurface, 0, 1) ||
            ImGui::SliderFloat("roughness", &materials[i].roughness, 0, 1) ||
            ImGui::SliderFloat("metallic", &materials[i].metallic, 0, 1) ||
            ImGui::SliderFloat("specular", &materials[i].specular, 0, 1) ||
            ImGui::SliderFloat("specularTint", &materials[i].specularTint, 0, 1)) {
            detail.frameIndex = 1;
            hasChanged = true;
        }
        ImGui::PopID();
    }
    ImGui::EndTabBar();
    if (hasChanged) {
        world->updateBuffer();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void loadCornellBox(World* world, const glm::vec3& pos, float boxLen = 3, float lightLen = 0.7) {
    glm::vec3 red = glm::vec3(.65, .05, .05);
    glm::vec3 green = glm::vec3(.12, .45, .15);
    glm::vec3 white = glm::vec3(1.0, 1.0, 1.0);

    Material m;
    m.roughness = 1.0;
    m.albedo = glm::vec3(0);
    m.emissionColor = {1, 1, 1};
    m.emissionStrength = 20;

    world->add<Quad>(
            { {pos.x + boxLen * 0.5 + lightLen * 0.5, pos.y + boxLen - 0.000001, pos.z - lightLen * 0.5 - lightLen}, {-lightLen, 0, 0}, {0, 0, -lightLen} },
            m, false 
        );

    m.emissionColor = {0, 0, 0};
    m.emissionStrength = 0;
 
    m.albedo = white;
    world->add<Quad>(
            { {pos.x, pos.y, pos.z}, {boxLen, 0, 0}, {0, 0, -boxLen} },
            m, false 
        );

    world->add<Quad>(
            { {pos.x + boxLen, pos.y + boxLen, pos.z}, {-boxLen, 0, 0}, {0, 0, -boxLen} },
            m, false 
        );

    world->add<Quad>(
            { {pos.x + boxLen, pos.y, pos.z}, {-boxLen, 0, 0}, {0, boxLen, 0} },
            m, false 
        );

    world->add<Quad>(
            { {pos.x, pos.y, pos.z - boxLen}, {boxLen, 0, 0}, {0, boxLen, 0} },
            m, false 
        );

    m.albedo = red;
    world->add<Quad>(
            { {pos.x, pos.y, pos.z - boxLen}, {0, boxLen, 0}, {0, 0, boxLen} },
            m, false 
        );

    m.albedo = green;
    world->add<Quad>(
            { {pos.x + boxLen, pos.y, pos.z}, {0, boxLen, 0}, {0, 0, -boxLen} },
            m, false 
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

    Material m;
    m.roughness = 1;
    m.albedo = glm::vec3(1);
    world->add<Quad>(
            { pos + b1Vertex[0], b1Vertex[4] - b1Vertex[0], b1Vertex[1] - b1Vertex[0]},
            m, false
        );

    world->add<Quad>(
            { pos + b1Vertex[1], b1Vertex[5] - b1Vertex[1], b1Vertex[2] - b1Vertex[1]},
            m, false 
        );

    world->add<Quad>(
            { pos + b1Vertex[2], b1Vertex[6] - b1Vertex[2], b1Vertex[3] - b1Vertex[2] },
            m, false 
        );

    world->add<Quad>(
            { pos + b1Vertex[3], b1Vertex[7] - b1Vertex[3], b1Vertex[0] - b1Vertex[3] },
            m, false 
        );

    world->add<Quad>(
            { pos + b1Vertex[4], b1Vertex[3] - b1Vertex[0], b1Vertex[1] - b1Vertex[0]},
            m, false 
        );
}

void loadScene1(World* world) {
    Material m;
    m.roughness = 1;
    m.albedo = {0.7, 0.7, 0.7};
    world->add<Sphere>(
            { 50, {0, -50 - 0.2, 1.2}, 0 },
            m, false
        );
    
    m.emissionColor = {1, 1, 1};
    m.emissionStrength = 30;
    m.albedo = {0, 0, 0};
    world->add<Sphere>(
            { 1, {0, 3, 15}, 1 },
            m, false
        );
    m.emissionColor = {0, 0, 0};
    m.emissionStrength = 0;

    m.albedo = {0.6, 0.3, 0.5};
    world->add<Sphere>(
            { 0.2, {0, 0, 1.2}, 2 },
            m, false
        );

    m.albedo = {0.3, 0.8, 0.5};
    world->add<Sphere>(
            { 0.2, {0.5, 0, 1.2}, 2 },
            m, false
        );
}

void loadScene2(World* world) {
    float boxLen = 3;
    float lightLen = 0.9;
    glm::vec3 pos = {-boxLen * 0.5, -boxLen * 0.5, 2 * boxLen};
    glm::vec3 center = pos + glm::vec3{boxLen, 0, -boxLen} * 0.5f;
    loadCornellBox(world, pos, boxLen, lightLen);

    Material m;
    m.roughness = 1;
    m.albedo = {1, 1, 1};

    world->add<Sphere>(
            { 0.5, center + glm::vec3{0, 1, 0}, 2 },
            m, false
        );
}

void loadScene3(World* world) {
    world->skyColor = { 0.0, 0.0, 0.0 };
    world->cam.fov = 54;

    float boxLen = 4;
    float lightLen = 1.2;
    glm::vec3 pos = {-boxLen * 0.5, -2, 2 * boxLen};
    loadCornellBox(world, pos, boxLen, lightLen);

    glm::vec2 b1Len = {1.2, 2.4};
    glm::vec3 b1Pos = {1.2, 0, 1.3};
    glm::vec3 b2Pos = {2.64, 0, 2.5};
    loadBox(world, b1Len, {pos.x + b1Pos.x, pos.y + b1Pos.y, pos.z - b1Pos.z}, 18);
    loadBox(world, {1.15, 1.15}, {pos.x + b2Pos.x, pos.y + b2Pos.y, pos.z - b2Pos.z}, -18);
}

void RoughnessMetallicTest(World *world) {
    Material m;
    m.roughness = 1.0;
    int groundLen = 10;
    world->add<Quad>(
            { {groundLen * 0.5, -0.1 - 1, groundLen * 0.5}, {0, 0, -groundLen}, {-groundLen, 0, 0} }, m, false );

    glm::vec3 red = glm::vec3(.65, .05, .05);
    m.albedo = red;
    m.specular = 1;
    for (int i = 0; i <= 10; ++i) {
        for (int j = 0; j < 2; ++j) {
            m.roughness = i / 10.0;
            m.metallic = j * (1 - i / 10.0);
            world->add<Sphere>(
                    { 0.1, {i * 0.3 - groundLen * 0.5 * 0.3, -1, 2 - j * 0.5} }, m, false);
        }
    }

    m.emissionColor = {1, 1, 1};
    m.emissionStrength = 300;
    m.albedo = {0, 0, 0};
    world->add<Sphere>(
            { 1, {-5, 8, -15}, 1 },
            m, false
        );
    m.emissionColor = {0, 0, 0};
    m.emissionStrength = 0;
}

void Application::run() {
    gl::ShaderProgram screenShader;
    screenShader.attach_shader(GL_VERTEX_SHADER, SHADER_SOURCE_DIRECTORY "screen.vert");
    screenShader.attach_shader(GL_FRAGMENT_SHADER, SHADER_SOURCE_DIRECTORY "screen.frag");
    screenShader.link();
    screenShader.bind();

    quad = new gl::Quad(vertices, indices);

    world = new World();

    world->cam.pos = glm::vec3(0);
    world->cam.forward = glm::vec3(0, 0, 1);

    GLCALL(glClearColor(0.1, 0.1, 0.1, 1));

    // loadScene1(world);
    // loadScene2(world);
    loadScene3(world);

    // RoughnessMetallicTest(world);

    glm::vec3 dispatchGroups = { (detail.resolution.x + 15) / 16, (detail.resolution.y + 15) / 16, 1 };

    gl::ComputeShader compute(SHADER_SOURCE_DIRECTORY "raytrace.comp", dispatchGroups);
    gl::Image2D screenImg(detail.resolution.x, detail.resolution.y);

    world->fetchBuffer();

    while(!glfwWindowShouldClose(m_window))
    {
        static double st;
        st = glfwGetTime();

        world->bindBuffer();
        dispatchGroups = { (detail.resolution.x + 15) / 16, (detail.resolution.y + 15) / 16, 1 };
        compute.updateGroups(dispatchGroups);
        compute.bind();
        screenImg.bind(0);
        compute.set_1u("frameIndex", detail.frameIndex);
        compute.set_3f("skyColor", world->skyColor);

        compute.set_1f("cam.fov", world->cam.fov);
        compute.set_3f("cam.position", world->cam.pos);
        compute.set_3f("cam.forward", world->cam.forward);
        compute.set_3f("cam.right", world->cam.right);
        compute.set_3f("cam.up", world->cam.up);

        compute.set_1i("cam.bounces", world->cam.bounces);
        compute.set_1i("cam.rayPerPixel", world->cam.rayPerPixel);

        compute.use();
        world->unbindBuffer();

        quad->vao.bind();
        screenShader.bind();
        screenImg.bindTexture(0);
        screenShader.set_2f("resolution", detail.resolution);
        screenShader.set_1i("tex", 0);
        screenShader.set_1i("toneMappingMethodIdx", detail.toneMappingMethodIdx);
        screenShader.set_1f("exposure", world->exposure);
        screenShader.set_1f("gamma", world->gamma);
        GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
        quad->vao.unbind();

        std::stringstream ss;
        ss << "render: " << (glfwGetTime() - st) * 1000.0 << "ms"
                        << " frameIndex: " << std::to_string(detail.frameIndex);
        glfwSetWindowTitle(m_window, ss.str().c_str());

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
            screenImg.resize(detail.resolution.x, detail.resolution.y);
            detail.frameIndex = 1;
        }
    }
}

