#version 330 core

uniform vec2 resolution;
uniform vec2 center;
uniform float time;
uniform float zoom;
uniform vec2 mouse_coord;

#define PI 3.1415f
vec2 uv = ((gl_FragCoord.xy / resolution - 0.5) * zoom - center) * vec2(1, resolution.y / resolution.x);
vec2 mouse = ((mouse_coord - 0.5) * zoom - center) * vec2(1, resolution.y / resolution.x);

layout(location = 0) out vec4 frag_color;

float line(vec2 p, vec2 p1, vec2 p2) {
    float m = (p1.y - p2.y) / (p1.x - p2.x);
    return m * (p.x - p1.x) - p.y + p1.y;
}

float distance_to_line(vec2 p, vec2 p1, vec2 p2) {
    float a = (p1.y - p2.y) / (p1.x - p2.x);
    float c = p2.y - p2.x * (p1.y - p2.y) / (p1.x - p2.x);
    if (p1.x == p2.x) {
        return abs(p1.x - p.x);
    }
    return abs((a * p.x - p.y + c)) / sqrt(a * a + 1);
}

vec2 lerp(vec2 a, vec2 b, float t) {
    return a + t * (b - a);
}

float sdf_circle(vec2 point, float r) {
    return length(point) - r;
}

void test() {
    float speed = time;
    float smoothness = 2 * zoom / resolution.x;
    float lw = 0.025;

    vec2 p1 = vec2(0, 0);
    vec2 p2 = vec2(0.5, 0.5);
    vec2 p3 = vec2(1, 0);

    float t = clamp(uv.x, 0, 1);
    vec2 a = lerp(p1, p2, t);
    vec2 b = lerp(p2, p3, t);

    vec3 color = vec3(0);
    //color += (1 - smoothstep(lw - smoothness * 0.5, lw + smoothness * 0.5, distance_to_line(uv, vec2(0), vec2(1, 0))));
    color += (1 - smoothstep(lw - smoothness * 0.5, lw + smoothness * 0.5, distance_to_line(uv, a, b)));
    if (step(0, uv.x - mouse.x) != 1) {
        color *= (1 - step(0, uv.x - mouse.x)) * vec3(uv.x, uv.y, 1);
    }

    frag_color = vec4(color, 1);
}

void test1() {
    float speed = time;
    float smoothness = 2 * zoom / resolution.x;
    float lw = 0.025;

    vec2 p1 = vec2(0, 0);
    vec2 p2 = vec2(0.5, 0.5);
    vec2 p3 = vec2(1, 0);

    float t = clamp(uv.x, 0, 1);
    vec2 a = lerp(p1, p2, t);
    vec2 b = lerp(p2, p3, t);

    vec3 color = vec3(0);
    //color += (1 - smoothstep(lw - smoothness * 0.5, lw + smoothness * 0.5, distance_to_line(uv, vec2(0), vec2(1, 0))));
    float offset = sin(time * 10) * 0.005;
    vec2 uvx = vec2(uv.x - offset, uv.y);
    vec2 uvy = vec2(uv.x + offset, uv.y);
    vec2 uvz = vec2(uv.x, uv.y + offset);
    float x = (1 - smoothstep(lw - smoothness * 0.5, lw + smoothness * 0.5, distance_to_line(uvx, a, b)));
    float y = (1 - smoothstep(lw - smoothness * 0.5, lw + smoothness * 0.5, distance_to_line(uvy, a, b)));
    float z = (1 - smoothstep(lw - smoothness * 0.5, lw + smoothness * 0.5, distance_to_line(uvz, a, b)));
    color = vec3(x, y, z);

    frag_color = vec4(color, 1);
}

void main() {
    test1();
    return;
    float speed = time;
    float smoothness = 2 * zoom / resolution.x;
    float lw = 0.025;

    //vec2 p1 = vec2(0.5, 0.5) + vec2(cos(time), sin(time)) * 0.5 - vec2(0, 0.5);
    vec2 p1 = vec2(0, 0);
    vec2 p2 = vec2(0.5, 0.5);
    vec2 p3 = vec2(1, 0);

    //float t = clamp(sin(time) * 0.5 + 0.5, 0, 1);
    float t = clamp(uv.x, 0, 1);
    vec2 a = lerp(p1, p2, t);
    vec2 b = lerp(p2, p3, t);

    vec3 color = vec3(0);
    color += (1 - smoothstep(lw - smoothness * 0.5, lw + smoothness * 0.5, distance_to_line(uv, a, b)));

    color += 1 - smoothstep(0.01 - smoothness * 0.5, 0.01 + smoothness * 0.5, abs(sdf_circle(uv - p1, 0.05)));
    color += 1 - smoothstep(0.01 - smoothness * 0.5, 0.01 + smoothness * 0.5, abs(sdf_circle(uv - p2, 0.05)));
    color += 1 - smoothstep(0.01 - smoothness * 0.5, 0.01 + smoothness * 0.5, abs(sdf_circle(uv - p3, 0.05)));

    color *=  vec3(uv.x, uv.y, 1) + vec3(0.5);

    color += (1 - min(step(0.01, distance_to_line(uv, vec2(0), vec2(0, 1))),
                     step(0.01, distance_to_line(uv, vec2(0), vec2(1, 0))))) * 0.3;
    frag_color = vec4(color, 1);
}

