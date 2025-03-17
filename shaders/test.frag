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

float bolw(float smoothness, float width, float value) {
    return smoothstep(width - smoothness * 0.5, width + smoothness * 0.5, value);
}

float ract(vec2 point, float width, float height) {
    vec2 d = abs(point) - vec2(width, height);
    return length(max(d, 0)) + min(max(d.x, d.y), 0);
}

float sdf_circle(vec2 point, float r) {
    return length(point) - r;
}

float sdf_sin(vec2 point, float p, float f, float o) {
    float m = p * f * cos(f * point.x + o);
    float a = -m;
    float c = m * point.x - p * sin(f * point.x + o);
    return (a * point.x + point.y + c) / sqrt(a * a + 1);
}

float sdf_cos(vec2 point, float p, float f, float o) {
    float m = p * f * -sin(f * point.x + o);
    float a = -m;
    float c = m * point.x - p * cos(f * point.x + o);
    return (a * point.x + point.y + c) / sqrt(a * a + 1);
}

float line(vec2 point, vec2 lpoint, float m) {
    float _px = lpoint.x;
    float _py = lpoint.y;
    float a = -m;
    float c = m * _px - _py;
    return (a * point.x + point.y + c) / sqrt(a * a + 1);
    //return m * (uv.x - point.x) - uv.y + point.y;
}

void test() {
    //float width = 0.02 * (sin(time) * 0.5 + 0.55) + 0.01;
    float width = 0.02;
    float smoothness = 0.02;
    float offset = time * 3;
    float p = 1 * (sin(time) * 0.5 + 0.5);
    float f = 4;
    float px = 0;
    float py = p * sin(f * px - offset);
    float m = p * cos(f * px - offset) * f;

    //float l = max(step(1, length(uv - vec2(0, p * sin(f * px - offset)))), abs(line(uv, vec2(px, py), m)));

    vec3 color = vec3(0);
    //color += 1 - bolw(smoothness, width, l);

    //float wave = sdf_sin(uv, p, f, -offset) + sdf_cos(uv, p, f, offset);
    float wave = sdf_sin(uv * 1.5, p, f * 0.5, -offset) * sdf_sin(uv, p * 0.8, 0, offset);
    color += 1 - bolw(smoothness, width, abs(wave));

    float cl = 1 - min(step(0.01, abs(line(uv, vec2(0, 0), 0))),
                       step(0.008, abs(line(vec2(uv.x, 0), vec2(0, 0), 1))));
    color += cl * 0.25;
    color += (1 - step(sin(time) * 0.5 + 0.5, ract(uv, 0.3, 0.3))) * 0.2;
    color.xy *= (uv + center) / zoom + 0.5;

    frag_color = vec4(color, 1);
}

void main() {
    {
        float b_0_1 = (sin(time) + 1) * 0.5 + 0.02;
        vec3 color = vec3(1 - smoothstep(0.05, 0.1, abs(uv.x * uv.x * uv.y * uv.y + uv.y * uv.y * uv.x *uv.x - 1 * b_0_1)));
        frag_color = vec4(color, 1);
    }
    return;
    //test();
    float width = 0.02;
    float smoothness = 1 / resolution.x;
    float offset = time * 3;
    float f = 2;
    float p = 1 * (sin(time * f) * 0.5 + 0.5);
    float px = 0;
    //float py = p * sin(f * px - offset);
    //float m = p * cos(f * px - offset) * f;

    vec3 color = vec3(0);
    vec2 point = uv;
    float o = offset * 3;
    float m =  1 * (cos(point.x * f - o) + sin(point.x * f + o));
    float py = 1 * (sin(point.x * f - o) - cos(point.x * f + o));
    float a = -m;
    float c = m * point.x - py;
    float wave = (a * point.x + point.y + c) / sqrt(a * a + 1);
    //color += (1 - bolw(smoothness, width, abs(wave)));

    vec2 pos = mouse;
    float angle = atan(pos.y / pos.x);
    mat2 rotate2d = mat2(cos(angle), -sin(angle),
                         sin(angle),  cos(angle));
    wave = sdf_sin(uv * 1.5, p, f, -offset) * sdf_sin(uv, p, 0, offset);
    color += 1 - smoothstep(0.02 - 0.0025, 0.02 + 0.0025, abs(wave));

    //float cc = abs(sdf_circle(uv, 0.5)) * sdf_circle(uv - vec2(sin(time) * 2, 0), 0.5) + sdf_circle(uv, 0.5) * abs(sdf_circle(uv - vec2(sin(time) * 2, 0), 0.5));
    //color += 1 - smoothstep(0.2 - 0.015, 0.2 + 0.015, cc);

    vec2 _pos = rotate2d * uv;
    //float cc = abs(ract(_pos, 0.5, 0.5)) * sdf_circle(uv - mouse, 0.5) +
    //           ract(_pos, 0.5, 0.5) * abs(sdf_circle(uv - mouse, 0.5));
    //color += 1 - smoothstep(0.2 - 0.02, 0.2, cc);
    float cc = ract(_pos, 0.5, 0.5);
    color += 1 - step(0, cc);

    float cl = 1 - min(step(0.01, abs(line(uv, vec2(0, 0), 0))),
                       step(0.008, abs(line(vec2(uv.x, 0), vec2(0, 0), 1))));
    color += cl * 0.25;
    color.xy *= (uv + center) / zoom + 0.5;
    frag_color = vec4(color, 1);
}
