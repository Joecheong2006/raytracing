#include "ShaderProgram.h"
#include "Renderer.h"
#include <fstream>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

ShaderProgram::ShaderProgram()
{
    GLCALL(m_id = glCreateProgram());
}

ShaderProgram::ShaderProgram(const ShaderProgram& shader) {
    m_id = shader.m_id;
    error = shader.error;
    m_uniform_location_cache = shader.m_uniform_location_cache;
    m_id = 0;
}

ShaderProgram::~ShaderProgram()
{
    release();
}

void ShaderProgram::create() {
    if (!m_id) {
        GLCALL(m_id = glCreateProgram());
        error.reserve(16);
    }
}

void ShaderProgram::attach_shader(u32 type, const std::string& path)
{
    u32 shader = compile_shader(shader_source(path), type);
    if (shader) {
        GLCALL(glAttachShader(m_id, shader));
        GLCALL(glDeleteShader(shader));
    }
}

void ShaderProgram::link()
{
    GLCALL(glLinkProgram(m_id));
}

void ShaderProgram::bind() const
{
    GLCALL(glUseProgram(m_id));
}

void ShaderProgram::unbind() const
{
    GLCALL(glUseProgram(0));
}

void ShaderProgram::release() {
    if (m_id) {
        GLCALL(glDeleteProgram(m_id));
        m_id = 0;
        m_uniform_location_cache = {};
    }
}

void ShaderProgram::set_4f(const std::string& name, f32 v0, f32 v1, f32 v2, f32 v3)
{
    int id = uniform_location(name);
    if (id != -1) {
        GLCALL(glUniform4f(id, v0, v1, v2, v3));
    }
}

void ShaderProgram::set_3f(const std::string& name, const glm::vec3& v) {
    set_3f(name, v[0], v[1], v[2]);
}

void ShaderProgram::set_3f(const std::string& name, f32 v0, f32 v1, f32 v2)
{
    int id = uniform_location(name);
    if (id != -1) {
        GLCALL(glUniform3f(id, v0, v1, v2));
    }
}

void ShaderProgram::set_3f(const std::string& name, f32* v)
{
    set_3f(name, v[0], v[1], v[2]);
}

void ShaderProgram::set_2f(const std::string& name, const glm::vec2& v) {
    set_2f(name, v[0], v[1]);
}

void ShaderProgram::set_2f(const std::string& name, f32* v) {
    set_2f(name, v[0], v[1]);
}

void ShaderProgram::set_2f(const std::string& name, f32 v0, f32 v1)
{
    int id = uniform_location(name);
    if (id != -1) {
        GLCALL(glUniform2f(id, v0, v1));
    }
}

void ShaderProgram::set_1u(const std::string& name, u32 v0) {
    int id = uniform_location(name);
    if (id != -1) {
        GLCALL(glUniform1ui(id, v0));
    }
}

void ShaderProgram::set_1i(const std::string& name, i32 v0)
{
    int id = uniform_location(name);
    if (id != -1) {
        GLCALL(glUniform1i(id, v0));
    }
}

void ShaderProgram::set_1f(const std::string& name, f32 v0)
{
    int id = uniform_location(name);
    if (id != -1) {
        GLCALL(glUniform1f(id, v0));
    }
}

void ShaderProgram::set_m4(const std::string& name, const glm::mat4& m)
{
    int id = uniform_location(name);
    if (id != -1) {
        GLCALL(glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(m)));
    }
}

u32 ShaderProgram::compile_shader(const std::string& source, u32 type)
{
    GLCALL(u32 id = glCreateShader(type));
    const char* src = source.c_str();
    GLCALL(glShaderSource(id, 1, &src, NULL));
    GLCALL(glCompileShader(id));

    int result;
    GLCALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if(result == GL_FALSE)
    {
        char infolog[512];
        GLCALL(glGetShaderInfoLog(id, 512, NULL, infolog));
        error.emplace_back(infolog);
        GLCALL(glDeleteShader(id));
        return 0;
    }

    return id;
}

const std::string ShaderProgram::shader_source(const std::string& path)
{
    std::ifstream stream(path);
    std::string line;
    std::string source;
    while(getline(stream, line))
        source += line + "\n";
    stream.close();
    return source;
}

i32 ShaderProgram::uniform_location(const std::string& name)
{
    if(m_uniform_location_cache.find(name) != m_uniform_location_cache.end())
        return m_uniform_location_cache[name];
    GLCALL(i32 location = glGetUniformLocation(m_id, name.c_str()));
    if (location != -1) {
        m_uniform_location_cache[name] = location;
    }
    return location;
}

