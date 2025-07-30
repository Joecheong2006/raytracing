#version 430 core

layout(location = 0) out vec4 fragColor;

in vec2 texCoord;


uniform vec2 resolution;
uniform sampler2D tex;

uniform int toneMappingMethodIdx;
uniform float exposure;
uniform float gamma;

#define AGX_LOOK 0

// Agx
vec3 tonemapAGX(vec3 x) {
    x = max(x, vec3(0.0));

    // AGX curve parameters
    const vec3 A = vec3(2.51, 2.51, 2.51);
    const vec3 B = vec3(0.03, 0.03, 0.03);
    const vec3 C = vec3(2.43, 2.43, 2.43);
    const vec3 D = vec3(0.59, 0.59, 0.59);
    const vec3 E = vec3(0.14, 0.14, 0.14);

    vec3 num = x * (A * x + B);
    vec3 den = x * (C * x + D) + E;

    vec3 mapped = num / den;

    // Highlight desaturation ("Abney effect" compensation)
    float luma = dot(mapped, vec3(0.2126, 0.7152, 0.0722));
    mapped = mix(vec3(luma), mapped, 0.6);

    return clamp(mapped, vec3(0.0), vec3(1.0));
}

vec3 applyAGXAndSRGB(vec3 hdr, float gamma) {
    vec3 tm = tonemapAGX(hdr);
    return pow(tm, vec3(1.0 / gamma));
}

//filmic
//https://github.com/dmnsgn/glsl-tone-map
// Filmic Tonemapping Operators http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 filmic(vec3 x) {
  vec3 X = max(vec3(0.0), x - 0.004);
  vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
  return pow(result, vec3(2.2));
}

//Lottes
//https://github.com/dmnsgn/glsl-tone-map/blob/main/lottes.glsl
// Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
vec3 lottes(vec3 x) {
  x *= vec3(0.9); //I reduced the light a little
  const vec3 a = vec3(1.6);
  const vec3 d = vec3(0.977);
  const vec3 hdrMax = vec3(8.0);
  const vec3 midIn = vec3(0.18);
  const vec3 midOut = vec3(0.267);

  const vec3 b =
	  (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
	  ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
  const vec3 c =
	  (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
	  ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

  return pow(x, a) / (pow(x, a * d) * b + c);
}

//pbr neutral
// https://github.com/KhronosGroup/ToneMapping
// https://github.com/KhronosGroup/ToneMapping/tree/main/PBR_Neutral
// Input color is non-negative and resides in the Linear Rec. 709 color space.
// Output color is also Linear Rec. 709, but in the [0, 1] range.
vec3 PBRNeutralToneMapping( vec3 color ) {
  const float startCompression = 0.8 - 0.04;
  const float desaturation = 0.15;

  float x = min(color.r, min(color.g, color.b));
  float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
  color -= offset;

  float peak = max(color.r, max(color.g, color.b));
  if (peak < startCompression) return color;

  const float d = 1. - startCompression;
  float newPeak = 1. - d * d / (peak + d - startCompression);
  color *= newPeak / peak;

  float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
  return mix(color, newPeak * vec3(1, 1, 1), g);
}

//uncharted
//you can change the Brightness variable if you want!
vec3 Uncharted2Tonemap(vec3 x) {
	float Brightness = 0.28;
	x*= Brightness;
	float A = 0.28;
	float B = 0.29;		
	float C = 0.10;
	float D = 0.2;
	float E = 0.025;
	float F = 0.35;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 unchartedTonemapping(vec3 color)
{
	vec3 curr = Uncharted2Tonemap(color*4.7);
	color = curr/Uncharted2Tonemap(vec3(15.2));	
	return color;
}

/////////////////////////////////////////////////////////////////////////////////////////
//ZCAM
//https://www.shadertoy.com/view/dlGBDD
//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
/////////////////////////////////////////////////////////////////////////////////////////



// eotf_pq parameters
const float Zcam_Lp = 10000.0;
const float Zcam_m1 = 2610.0 / 16384.0;
const float Zcam_m2 = 1.7 * 2523.0 / 32.0;
const float Zcam_c1 = 107.0 / 128.0;
const float Zcam_c2 = 2413.0 / 128.0;
const float Zcam_c3 = 2392.0 / 128.0;

vec3 eotf_pq(vec3 x)
{
	x = sign(x) * pow(abs(x), vec3(1.0 / Zcam_m2));
	x = sign(x) * pow((abs(x) - Zcam_c1) / (Zcam_c2 - Zcam_c3 * abs(x)), vec3(1.0 / Zcam_m1)) * Zcam_Lp;
	return x;
}

vec3 eotf_pq_inverse(vec3 x)
{
	x /= Zcam_Lp;
	x = sign(x) * pow(abs(x), vec3(Zcam_m1));
	x = sign(x) * pow((Zcam_c1 + Zcam_c2 * abs(x)) / (1.0 + Zcam_c3 * abs(x)), vec3(Zcam_m2));
	return x;
}

// XYZ <-> ICh parameters
const float Zcam_W = 140.0;
const float Zcam_b = 1.15;
const float Zcam_g = 0.66;

vec3 XYZ_to_ICh(vec3 XYZ)
{
	XYZ *= Zcam_W;
	XYZ.xy = vec2(Zcam_b, Zcam_g) * XYZ.xy - (vec2(Zcam_b, Zcam_g) - 1.0) * XYZ.zx;
	
	const mat3 XYZ_to_LMS = transpose(mat3(
		 0.41479,   0.579999, 0.014648,
		-0.20151,   1.12065,  0.0531008,
		-0.0166008, 0.2648,   0.66848));
	
	vec3 LMS = XYZ_to_LMS * XYZ;
	LMS = eotf_pq_inverse(LMS);
	
	const mat3 LMS_to_Iab = transpose(mat3(
		0.0,       1.0,      0.0,
		3.524,    -4.06671,  0.542708,
		0.199076,  1.0968,  -1.29588));
	
	vec3 Iab = LMS_to_Iab * LMS;
	
	float I = eotf_pq(vec3(Iab.x)).x / Zcam_W;
	float C = length(Iab.yz);
	float h = atan(Iab.z, Iab.y);
	return vec3(I, C, h);
}

vec3 ICh_to_XYZ(vec3 ICh)
{
	vec3 Iab;
	Iab.x = eotf_pq_inverse(vec3(ICh.x * Zcam_W)).x;
	Iab.y = ICh.y * cos(ICh.z);
	Iab.z = ICh.y * sin(ICh.z);
	
	const mat3 Iab_to_LMS = transpose(mat3(
		1.0, 0.2772,  0.1161,
		1.0, 0.0,     0.0,
		1.0, 0.0426, -0.7538));
	
	vec3 LMS = Iab_to_LMS * Iab;
	LMS = eotf_pq(LMS);
	
	const mat3 LMS_to_XYZ = transpose(mat3(
		 1.92423, -1.00479,  0.03765,
		 0.35032,  0.72648, -0.06538,
		-0.09098, -0.31273,  1.52277));
	
	vec3 XYZ = LMS_to_XYZ * LMS;
	XYZ.x = (XYZ.x + (Zcam_b - 1.0) * XYZ.z) / Zcam_b;
	XYZ.y = (XYZ.y + (Zcam_g - 1.0) * XYZ.x) / Zcam_g;
	return XYZ / Zcam_W;
}

const mat3 XYZ_to_sRGB = transpose(mat3(
	 3.2404542, -1.5371385, -0.4985314,
	-0.9692660,  1.8760108,  0.0415560,
	 0.0556434, -0.2040259,  1.0572252));

const mat3 sRGB_to_XYZ = transpose(mat3(
	0.4124564, 0.3575761, 0.1804375,
	0.2126729, 0.7151522, 0.0721750,
	0.0193339, 0.1191920, 0.9503041));

bool in_sRGB_gamut(vec3 ICh)
{
	vec3 sRGB = XYZ_to_sRGB * ICh_to_XYZ(ICh);
	return all(greaterThanEqual(sRGB, vec3(0.0))) && all(lessThanEqual(sRGB, vec3(1.0)));
}

vec3 Zcam_tonemap(vec3 sRGB)
{	
	vec3 ICh = XYZ_to_ICh(sRGB_to_XYZ * sRGB);
	
	const float s0 = 0.71;
	const float s1 = 1.04;
	const float p = 1.40;
	const float t0 = 0.01;
	float n = s1 * pow(ICh.x / (ICh.x + s0), p);
	ICh.x = clamp(n * n / (n + t0), 0.0, 1.0);
	
	if (!in_sRGB_gamut(ICh))
	{
		float C = ICh.y;
		ICh.y -= 0.5 * C;
		
		for (float i = 0.25; i >= 1.0 / 256.0; i *= 0.5)
		{
			ICh.y += (in_sRGB_gamut(ICh) ? i : -i) * C;
		}
	}
	
	return XYZ_to_sRGB * ICh_to_XYZ(ICh);
}

/////////////////////////////////////////////////////////////////////////////////////////
//^ZCAM^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/////////////////////////////////////////////////////////////////////////////////////////


//

//https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
/*
=================================================================================================

  Baking Lab
  by MJP and David Neubelt
  http://mynameismjp.wordpress.com/

  All code licensed under the MIT license

=================================================================================================
 The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
 credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)
*/

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3x3 ACESInputMat = mat3x3
(
	0.59719, 0.35458, 0.04823,
	0.07600, 0.90834, 0.01566,
	0.02840, 0.13383, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3x3 ACESOutputMat = mat3x3
(
	 1.60475, -0.53108, -0.07367,
	-0.10208,  1.10813, -0.00605,
	-0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

vec3 ACESFitted(vec3 color)
{
	color = transpose(ACESInputMat) * color;
	// Apply RRT and ODT
	color = RRTAndODTFit(color);
	color = transpose(ACESOutputMat) * color;
	color = clamp(color, 0, 1);
	return color;
}

vec3 ACESFilmSimple(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0, 1);

}
//


// https://advances.realtimerendering.com/s2021/jpatry_advances2021/index.html
// https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.40.9608&rep=rep1&type=pdf
vec4 rgb_to_lmsr(vec3 c)
{
	const mat4x3 m = mat4x3(
		0.31670331, 0.70299344, 0.08120592, 
		0.10129085, 0.72118661, 0.12041039, 
		0.01451538, 0.05643031, 0.53416779, 
		0.01724063, 0.60147464, 0.40056206);
	return c * m;
}
vec3 lms_to_rgb(vec3 c)
{
	const mat3 m = mat3(
		 4.57829597, -4.48749114,  0.31554848, 
		-0.63342362,  2.03236026, -0.36183302, 
		-0.05749394, -0.09275939,  1.90172089);
	return c * m;
}

void main() {
    vec3 color = texture(tex, texCoord).rgb;

    if (any(isnan(color)) || any(isinf(color)) || any(lessThan(color, vec3(0.0)))) {
        fragColor = vec4(0.0, 2.0, 0.0, 1.0);
        return;
    }

    // Tone Mapping
    if (toneMappingMethodIdx == 1) { // Agx
        color = applyAGXAndSRGB(color * exposure, gamma);
    }
    else if (toneMappingMethodIdx == 2) { // Reinhard
        color = (color * exposure) / (color * exposure + vec3(1.0));
        color = pow(color, vec3(1.0 / gamma));
    }
    else if (toneMappingMethodIdx == 3) { // Filmic
        color = filmic(color * exposure);
        color = pow(color, vec3(1.0 / gamma));
    }
    else if (toneMappingMethodIdx == 4) { // Lottes
        color = lottes(color * exposure);
        color = pow(color, vec3(1.0 / gamma));
    }
    else if (toneMappingMethodIdx == 5) { // PBRneutral
        color = PBRNeutralToneMapping(color * exposure);
        color = pow(color, vec3(1.0 / gamma));
    }
    else if (toneMappingMethodIdx == 6) { // Uncharted
        color = unchartedTonemapping(color * exposure);
        color = pow(color, vec3(1.0 / gamma));
    }
    else if (toneMappingMethodIdx == 7) { // Zcam
        color = Zcam_tonemap(color * exposure);
        color = pow(color, vec3(1.0 / gamma));
    }
    else if (toneMappingMethodIdx == 8) { // Aces
        color = ACESFitted(color * exposure);
        color = pow(color, vec3(1.0 / gamma));
    }
    else {
        color = pow(color, vec3(1.0 / gamma));
    }
    
    fragColor = vec4(color, 1);
}

