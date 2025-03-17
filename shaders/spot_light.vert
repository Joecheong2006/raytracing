#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 v_tex_coord;
out vec3 fragPos;
out vec3 normal;

void main()
{
    vec4 pos = projection * view * model * vec4(position, 1);
    v_tex_coord = tex_coord;
    fragPos = vec3(model * vec4(position, 1));
    normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = pos;
}
