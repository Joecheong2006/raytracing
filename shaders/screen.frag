#version 430 core

layout(location = 0) out vec4 frag_color;

in vec2 texCoord;

uniform vec2 resolution;
uniform sampler2D tex;

uniform float exposure;
uniform float gamma;

/* ---------- AgX core ------------- */
const mat3 LINEAR_REC2020_TO_LINEAR_SRGB = mat3(
     1.6605, -0.1246, -0.0182,
    -0.5876,  1.1329, -0.1006,
    -0.0728, -0.0083,  1.1187);

const mat3 LINEAR_SRGB_TO_LINEAR_REC2020 = mat3(
    0.6274, 0.0691, 0.0164,
    0.3293, 0.9195, 0.0880,
    0.0433, 0.0113, 0.8956);

const mat3 AgXInsetMatrix = mat3(
    0.8566271533, 0.1373189729, 0.1118982130,
    0.0951212405, 0.7612419906, 0.0767994186,
    0.0482516061, 0.1014390365, 0.8113023684);

const mat3 AgXOutsetMatrix = mat3(
     1.1271005818, -0.1413297635, -0.1413297635,
    -0.1106066431,  1.1578237022, -0.1106066431,
    -0.0164939387, -0.0164939387,  1.2519364066);

const float AgxMinEv = -12.47393;
const float AgxMaxEv =   4.026069;

vec3 agxAscCdl(vec3 c, vec3 slope, vec3 offset, vec3 power, float sat)
{
    const vec3 LUMA = vec3(0.2126, 0.7152, 0.0722);
    float luma = dot(c, LUMA);
    vec3  graded = pow(c * slope + offset, power);
    return luma + sat * (graded - luma);
}

vec3 agx(vec3 color, int look)
{
    // 0. Optional global exposure (before anything else!)
    color *= exposure;

    // 1. Move from linear sRGB into wide-gamut Rec.2020
    color = LINEAR_SRGB_TO_LINEAR_REC2020 * color;

    // 2. Primary “inset” to stop out-of-gamut artefacts
    color = AgXInsetMatrix * color;
    color = max(color, 1e-10);               // avoid log(0)

    // 3. Encode to log2 EV space and normalise to 0-1
    color  = clamp(log2(color), AgxMinEv, AgxMaxEv);
    color  = (color - AgxMinEv) / (AgxMaxEv - AgxMinEv);

    // 4. 6-term polynomial approximation of the reference sigmoid
    vec3 x2 = color * color;
    vec3 x4 = x2 * x2;
    color =  15.5   * x4 * x2
           - 40.14  * x4 * color
           + 31.96  * x4
           -  6.868 * x2 * color
           +  0.4298* x2
           +  0.1191* color
           -  0.00232;

    // 5. Optional creative look
    if (look == 1) {        // “Golden”
        color = agxAscCdl(color,
                          vec3(1.0, 0.9, 0.5),   // slope
                          vec3(0.0),             // offset
                          vec3(0.8),             // power
                          1.3);                  // saturation
    } else if (look == 2) { // “Punchy”
        color = agxAscCdl(color,
                          vec3(1.0),             // slope
                          vec3(0.0),             // offset
                          vec3(1.35),            // power
                          1.4);
    }

    // 6. Bring back to Rec.2020 and linear space
    color = AgXOutsetMatrix * color;

    // 7. Apply a simple 2.2 display gamma (EOTF)
    color = pow(max(color, 0.0), vec3(2.2));

    // 8. Back to linear sRGB for a standard 8-bit window
    color = LINEAR_REC2020_TO_LINEAR_SRGB * color;

    // 9. Hard clamp (could be replaced with a gamut mapper)
    return clamp(color, 0.0, 1.0);
}

void main() {
    // frag_color = vec4(colors[int(gl_FragCoord.x + gl_FragCoord.y * resolution.x)], 1);
    vec3 color = texture(tex, texCoord).rgb;
    color = agx(color, 0);
    color = pow(color, vec3(1.0 / gamma));
    frag_color = vec4(color, 1);
}

