#version 430 core

#define PI 3.1415926
#define RAD(x) ((x) * PI / 180.0)

layout(location = 0) out vec4 frag_color;

uniform vec2 resolution;
uniform float frameIndex;
uniform float time;

struct camera {
    float fov;
    vec3 position;
    vec3 forward, right, up;
};

uniform camera cam;
uniform int bounces;

struct hit_info {
    vec3 point, normal;
    float t;
    int objId;
};

struct circle {
    vec3 center, color;
    float radius;
};

#define CIRCLE_LEN 2
circle circles[CIRCLE_LEN];

uint pcg(uint v) {
    uint state = v * uint(747796405) + uint(2891336453);
    uint word = ((state >> ((state >> uint(28)) + uint(4))) ^ state) * uint(277803737);
    return (word >> uint(22)) ^ word;
}

float rand(inout float seed) {
    seed = float(pcg(uint(seed)));
    return seed / float(uint(0xffffffff));
}

float randND(inout float seed) {
    float theta = 2 * PI * rand(seed);
    float rho = sqrt(-2 * log(rand(seed)));
    return rho * cos(theta);
}

vec2 rand2(inout float seed) {
    return vec2(randND(seed), randND(seed)); 
}

vec3 rand3(inout float seed) {
    return vec3(randND(seed), randND(seed), randND(seed)); 
}

struct ray {
    vec3 origin, direction;
};

ray Ray(vec3 origin, vec3 direction) {
    ray result;
    result.origin = origin;
    result.direction = normalize(direction);
    return result;
}

vec3 ray_at(ray r, float t) {
    return r.origin + t * r.direction;
}

vec3 reflect(vec3 v, vec3 n) {
    return v - dot(v, n) * n * 2.0;
}

vec3 rand_on_hemisphere(vec3 normal, float seed) {
    vec3 rv = rand3(seed);

    vec3 right = normalize(cross(rv, normal));
    vec3 forward = cross(right, normal);

    rv.x = randND(seed);
    rv.z = randND(seed);
    rv.y = (randND(seed) + 1) * 0.5;

    return normalize(rv.x * right + rv.y * normal + rv.z * forward);
}

bool hit_sphere_circle(in circle cir, ray r, float max, inout hit_info info) {
    vec3 dir = cir.center - r.origin;
    float a = dot(r.direction, r.direction);
    float b = -2.0 * dot(r.direction, dir);
    float c = dot(dir, dir) - cir.radius * cir.radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return false;
    }

    float sqrtd = sqrt(discriminant);
    info.t = (-b - sqrtd) / (2.0 * a);

    if (!(info.t > 0 && info.t < max)) {
        info.t = (-b + sqrtd) / (2.0 * a);
        if (!(info.t > 0 && info.t < max)) {
            return false;
        }
    }

    info.point = ray_at(r, info.t);
    info.normal = (info.point - cir.center) / cir.radius;
    if (dot(r.direction, info.normal) > 0) {
        info.normal = -info.normal;
    }
    return true;
}

bool hit(ray r, out hit_info track) {
    hit_info tmp;

    float closest = 0xffffff;
    bool hit_something = false;

    for (int i = 0; i < CIRCLE_LEN; ++i) {
        tmp.objId = i;
        if (hit_sphere_circle(circles[i], r, closest, tmp)) {
            hit_something = true;
            closest = tmp.t;
            track = tmp;
        }
    }

    return hit_something;
}

vec3 trace_color(ray r) {
    vec3 color = vec3(0.0);

    float m = 1.0;

    vec3 light = normalize(-vec3(1, 1, -1));
    // vec3 light = normalize(vec3(cos(time), 0, sin(time)));

    float seed = frameIndex + (gl_FragCoord.x * gl_FragCoord.y);
    for (int i = 0; i < bounces; ++i) {
        seed += i;
        hit_info info;
        if (hit(r, info)) {
            float lightIntensity = max(dot(-light, info.normal), 0) * m;
            color += circles[info.objId].color * lightIntensity;

            m *= 0.5;
            float roughness = 0.1;

            r.origin = info.point + info.normal * 0.0001;

            // r.direction = info.normal + normalize(rand3(seed)) * rand(seed);
            // r.direction = rand_on_hemisphere(info.normal, seed) * rand(seed);
            r.direction = reflect(r.direction, info.normal + rand3(seed) * 0.5 * roughness);
        }
        else {
            float a = (r.direction.y + 1) * 0.5;
            vec3 skyColor = (1.0 - a) * vec3(1) + a * vec3(0.5, 0.7, 1);
            // vec3 skyColor = vec3(0);
            return color + skyColor * m;
        }
    }

    return color;
}

void main() {
    circles[1].center = vec3(0, 0, 1.2);
    circles[1].color = vec3(0.3, 0.3, 0.1);
    circles[1].radius = 0.2;

    circles[0].center = vec3(0, -50 - circles[1].radius, 1.2);
    circles[0].color = vec3(0, 0.3, 0.3);
    circles[0].radius = 50;

    vec3 lookat = cam.forward + cam.position;
    vec3 camera_center = cam.position;

    float viewport_ratio = resolution.x / resolution.y;
    float focal_length = length(lookat - camera_center);
    float fov = cam.fov;

    float viewport_height = 2.0 * tan(RAD(fov) / 2.0) * focal_length;
    float viewport_width = viewport_height * viewport_ratio;
    vec2 viewport = vec2(viewport_width, viewport_height);

    vec3 k = normalize(lookat - camera_center);
    vec3 i = normalize(cross(-k, vec3(0.0, 1.0, 0.0)));
    vec3 j = cross(-i, k);
    k = cam.forward;
    i = cam.right;
    j = cam.up;

    vec3 uv = vec3(gl_FragCoord.xy / resolution * 2.0 - 1.0, 0);
    uv = viewport_width * 0.5 * i * uv.x + viewport_height * 0.5 * j * uv.y + focal_length * k + camera_center;

    vec2 per_pixel = viewport / vec2(resolution.x, resolution.y);
    float s = 0;

    // Random ray
    ray r = Ray(camera_center, uv + ((per_pixel.x + randND(s) / resolution.x / 2) * i +
                                     (per_pixel.y + randND(s) / resolution.y / 2) * j) * 0.5
                                    - camera_center);

    vec3 color = trace_color(r);
    frag_color = vec4(color, 1);
}

