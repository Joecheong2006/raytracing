#version 430 core

#define PI 3.1415926
#define RAD(x) ((x) * PI / 180.0)

layout(std430, binding = 0) buffer Screen {
    vec4 color[];
} screen;

struct Material {
    vec3 albedo;
    float roughness;

    vec3 emissionColor;
    float emissionRate;
};

struct Sphere {
    float radius;
    vec3 center;
    int materialIndex;
};

layout(std430, binding = 1) readonly buffer Materials {
    Material materials[];
};

layout(std430, binding = 2) readonly buffer Objects {
    Sphere spheres[];
} obj;

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

struct HitInfo {
    vec3 point, normal;
    float t;
    int objId;
};

uint pcg(uint v) {
    uint state = v * uint(747796405) + uint(2891336453);
    uint word = ((state >> ((state >> uint(28)) + uint(4))) ^ state) * uint(277803737);
    return (word >> uint(22)) ^ word;
}

float rand(inout double seed) {
    seed = double(pcg(uint(seed)));
    return float(seed) / float(uint(0xffffffff));
}

float randND(inout double seed) {
    float theta = 2 * PI * rand(seed);
    float rho = sqrt(-2 * log(rand(seed)));
    return rho * cos(theta);
}

vec2 rand2(inout double seed) {
    return vec2(randND(seed), randND(seed)); 
}

vec3 rand3(inout double seed) {
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

vec3 rayAt(ray r, float t) {
    return r.origin + t * r.direction;
}

vec3 reflect(vec3 v, vec3 n) {
    return v - dot(v, n) * n * 2.0;
}

vec3 randOnHemisphere(vec3 normal, inout double seed) {
    vec3 rv = rand3(seed);

    vec3 right = normalize(cross(rv, normal));
    vec3 forward = cross(right, normal);

    rv.x = randND(seed);
    rv.z = randND(seed);
    rv.y = (randND(seed) + 1) * 0.5;

    return normalize(rv.x * right + rv.y * normal + rv.z * forward);
}

bool hitSphere(in Sphere cir, ray r, float max, inout HitInfo info) {
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

    info.point = rayAt(r, info.t);
    info.normal = (info.point - cir.center) / cir.radius;
    if (dot(r.direction, info.normal) > 0) {
        info.normal = -info.normal;
    }
    return true;
}

void hit(ray r, out HitInfo track) {
    HitInfo tmp;

    float closest = 0xffffff;

    for (int i = 0; i < obj.spheres.length(); ++i) {
        tmp.objId = i;
        if (hitSphere(obj.spheres[i], r, closest, tmp)) {
            closest = tmp.t;
            track = tmp;
        }
    }

    if (closest == 0xffffff) {
        track.t = -1;
    }
}

vec3 traceColor(ray r, inout double seed) {
    vec3 incomingLight = vec3(0.0);

    vec3 rayColor = vec3(1);

    for (int i = 0; i < bounces; ++i) {
        seed += i;

        HitInfo info;
        hit(r, info);

        if (info.t < 0.0f) {
            float t = (r.direction.y + 1) * 0.5;
            vec3 skyColor = (1.0 - t) * vec3(1) + t * vec3(0.5, 0.7, 1);
            // skyColor = vec3(0);
            return incomingLight + skyColor * rayColor; 
        }

        r.origin = info.point + info.normal * 0.0001;
        r.direction = normalize(info.normal + normalize(rand3(seed)));

        int materialIndex = obj.spheres[info.objId].materialIndex;

        incomingLight += materials[materialIndex].emissionColor * materials[materialIndex].emissionRate * rayColor;
        rayColor *= materials[materialIndex].albedo * dot(info.normal, r.direction);
    }

    return incomingLight;
}

void main() {
    // Setup
    vec3 lookat = cam.forward + cam.position;
    vec3 cameraCenter = cam.position;

    float viewportRatio = resolution.x / resolution.y;
    float focalLength = length(lookat - cameraCenter);
    float fov = cam.fov;

    float viewportHeight = 2.0 * tan(RAD(fov) / 2.0) * focalLength;
    float viewportWidth = viewportHeight * viewportRatio;
    vec2 viewport = vec2(viewportWidth, viewportHeight);

    vec3 uv = vec3(gl_FragCoord.xy / resolution * 2.0 - 1.0, 0);
    uv = viewportWidth * 0.5 * uv.x * cam.right
       + viewportHeight * 0.5 * uv.y * cam.up
       + focalLength * cam.forward
       + cameraCenter;

    vec2 perPixel = viewport / vec2(resolution.x, resolution.y);
    double seed = double(frameIndex * (gl_FragCoord.x + gl_FragCoord.y * resolution.x));

    // Random ray at pixel center
    ray r = Ray(cameraCenter, uv + ((perPixel.x + randND(seed) / resolution.x) * cam.right +
                                    (perPixel.y + randND(seed) / resolution.y) * cam.up) * 0.5
                                    - cameraCenter);

    vec3 color = vec3(0.0);
    int rayPerPixel = 1;
    for (int i = 0; i < rayPerPixel; ++i) {
        color += traceColor(r, seed);
    }
    color /= float(rayPerPixel);

    int pos = int(gl_FragCoord.x + gl_FragCoord.y * resolution.x);
    screen.color[pos] = (screen.color[pos] * (float(frameIndex) - 1.0) + vec4(color, 1)) / float(frameIndex);
}

