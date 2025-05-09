#version 430 core

layout(location = 0) out vec4 frag_color;

in vec2 texCoord;

uniform vec2 resolution;
uniform sampler2D tex;

void main() {
    // frag_color = vec4(colors[int(gl_FragCoord.x + gl_FragCoord.y * resolution.x)], 1);
    frag_color = vec4(texture(tex, texCoord));
}

