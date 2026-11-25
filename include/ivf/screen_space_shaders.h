#pragma once

#include <string>

namespace ivf {

/**
 * @file screen_space_shaders.h
 * @brief GLSL of the screen-space effects (SsaoEffect, ScreenSpaceReflectionEffect).
 *
 * Both work from the scene's depth buffer only: positions are reconstructed with
 * `invProjection`, normals from neighbouring depths (the side with the smaller depth step,
 * so silhouettes do not bend the normals).
 */

/// Position and normal reconstruction from the depth buffer (pasted into the shaders).
inline const std::string screen_space_common_glsl = R"(
uniform sampler2D depthTexture;
uniform mat4 invProjection;
uniform mat4 projection;

float ivfDepthAt(vec2 uv)
{
    return texture(depthTexture, uv).r;
}

vec3 ivfViewPosFromDepth(vec2 uv, float depth)
{
    vec4 p = invProjection * vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    return p.xyz / p.w;
}

vec3 ivfViewPosAt(vec2 uv)
{
    return ivfViewPosFromDepth(uv, ivfDepthAt(uv));
}

// View-space normal of the surface at uv (pointing towards the camera)
vec3 ivfViewNormalAt(vec2 uv, vec3 p)
{
    vec2 texel = 1.0 / vec2(textureSize(depthTexture, 0));
    vec3 r = ivfViewPosAt(uv + vec2(texel.x, 0.0)) - p;
    vec3 l = p - ivfViewPosAt(uv - vec2(texel.x, 0.0));
    vec3 u = ivfViewPosAt(uv + vec2(0.0, texel.y)) - p;
    vec3 d = p - ivfViewPosAt(uv - vec2(0.0, texel.y));
    vec3 dx = abs(r.z) < abs(l.z) ? r : l;
    vec3 dy = abs(u.z) < abs(d.z) ? u : d;
    return normalize(cross(dx, dy));
}

vec2 ivfProject(vec3 viewPos, out float w)
{
    vec4 clip = projection * vec4(viewPos, 1.0);
    w = clip.w;
    return clip.xy / clip.w * 0.5 + 0.5;
}

// Per-pixel value in [0, 1) without texture (interleaved gradient noise)
float ivfPixelNoise()
{
    return fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
}
)";

/// SSAO pass 1: ambient occlusion from the depth buffer into alpha (colour copied to rgb).
inline const std::string ssao_frag_shader_source = std::string(R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
)") + screen_space_common_glsl + R"(
uniform float ssaoRadius = 0.5;   // world units
uniform float ssaoBias = 0.025;   // world units
uniform float ssaoPower = 1.5;
uniform int ssaoSamples = 16;

const int MAX_SAMPLES = 64;
const float GOLDEN_ANGLE = 2.39996323;

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;
    float depth = ivfDepthAt(TexCoords);
    if (depth >= 1.0)
    {
        FragColor = vec4(color, 1.0); // background: not occluded
        return;
    }

    vec3 p = ivfViewPosFromDepth(TexCoords, depth);
    vec3 n = ivfViewNormalAt(TexCoords, p);

    // Basis around the normal
    vec3 helper = abs(n.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 t = normalize(cross(helper, n));
    vec3 b = cross(n, t);

    // Cosine-weighted hemisphere samples on a golden-angle spiral, turned per pixel
    float noise = ivfPixelNoise();
    int count = clamp(ssaoSamples, 1, MAX_SAMPLES);
    float occlusion = 0.0;
    for (int i = 0; i < MAX_SAMPLES; ++i)
    {
        if (i >= count)
            break;
        float f = (float(i) + 0.5) / float(count);
        float phi = float(i) * GOLDEN_ANGLE + noise * 6.2831853;
        float sinTheta = sqrt(f);
        vec3 dir = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, sqrt(1.0 - f));
        // More samples close to the point
        float s = fract(float(i) * 0.618034 + noise);
        float scale = mix(0.1, 1.0, s * s);
        vec3 samplePos = p + (t * dir.x + b * dir.y + n * dir.z) * ssaoRadius * scale;

        float w;
        vec2 uv = ivfProject(samplePos, w);
        if (w <= 0.0 || uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            continue;
        float sceneDepth = ivfDepthAt(uv);
        if (sceneDepth >= 1.0)
            continue;
        float sceneZ = ivfViewPosFromDepth(uv, sceneDepth).z;
        // Occluded if the surface there is in front of the sample; ignore far-away surfaces
        float range = smoothstep(0.0, 1.0, ssaoRadius / max(abs(p.z - sceneZ), 1e-4));
        occlusion += (sceneZ >= samplePos.z + ssaoBias ? 1.0 : 0.0) * range;
    }

    float ao = pow(clamp(1.0 - occlusion / float(count), 0.0, 1.0), ssaoPower);
    FragColor = vec4(color, ao);
}
)";

/// SSAO pass 2: depth-aware blur of the occlusion (removes the noise pattern), applied to the colour.
inline const std::string ssao_apply_frag_shader_source = std::string(R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture; // rgb colour, a occlusion
)") + screen_space_common_glsl + R"(
uniform float ssaoIntensity = 1.0;
uniform int ssaoBlurRadius = 2;
uniform bool ssaoShow = false;

void main()
{
    vec4 center = texture(screenTexture, TexCoords);
    float depth = ivfDepthAt(TexCoords);
    float ao = center.a;
    if (depth < 1.0)
    {
        float z = ivfViewPosFromDepth(TexCoords, depth).z;
        vec2 texel = 1.0 / vec2(textureSize(screenTexture, 0));
        int r = clamp(ssaoBlurRadius, 0, 4);
        float sum = 0.0;
        float weights = 0.0;
        for (int x = -r; x <= r; ++x)
        {
            for (int y = -r; y <= r; ++y)
            {
                vec2 uv = TexCoords + vec2(x, y) * texel;
                float d = ivfDepthAt(uv);
                if (d >= 1.0)
                    continue;
                // Only neighbours at about the same depth: edges stay sharp
                float dz = abs(ivfViewPosFromDepth(uv, d).z - z) / max(-z, 1e-3);
                float w = 1.0 / (1.0 + 400.0 * dz * dz + 50.0 * dz);
                sum += texture(screenTexture, uv).a * w;
                weights += w;
            }
        }
        ao = weights > 0.0 ? sum / weights : ao;
    }
    ao = mix(1.0, ao, clamp(ssaoIntensity, 0.0, 1.0));

    FragColor = ssaoShow ? vec4(vec3(ao), 1.0) : vec4(center.rgb * ao, 1.0);
}
)";

/// Screen-space reflections: march the reflected ray through the depth buffer and take the
/// colour where it hits, from the image's mip chain for rough surfaces.
inline const std::string ssr_frag_shader_source = std::string(R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int screenTextureLevels; // declared: the post-processor builds a mip chain
uniform mat4 invView;
)") + screen_space_common_glsl + R"(
uniform float ssrStrength = 0.6;
uniform float ssrMaxDistance = 20.0; // world units
uniform int ssrSteps = 48;
uniform float ssrThickness = 0.5;    // world units
uniform float ssrRoughness = 0.0;    // 0 mirror .. 1 blurred
uniform float ssrFresnel = 0.5;      // 0 uniform .. 1 only at grazing angles
uniform float ssrEdgeFade = 0.1;     // fraction of the screen
uniform float ssrSurfaceCos = 0.9;   // cosine of the largest angle between normal and ssrUp
uniform vec3 ssrUp = vec3(0.0, 1.0, 0.0);
uniform bool ssrShow = false;

const int MAX_STEPS = 128;

void main()
{
    vec4 color = textureLod(screenTexture, TexCoords, 0.0);
    float depth = ivfDepthAt(TexCoords);
    if (depth >= 1.0)
    {
        FragColor = ssrShow ? vec4(0.0, 0.0, 0.0, 1.0) : color;
        return;
    }

    vec3 p = ivfViewPosFromDepth(TexCoords, depth);
    vec3 n = ivfViewNormalAt(TexCoords, p);

    // Only surfaces facing ssrUp reflect (floors, water, ...)
    float facing = dot(normalize(mat3(invView) * n), normalize(ssrUp));
    float mask = smoothstep(ssrSurfaceCos, mix(ssrSurfaceCos, 1.0, 0.5), facing);
    if (mask <= 0.0)
    {
        FragColor = ssrShow ? vec4(0.0, 0.0, 0.0, 1.0) : color;
        return;
    }

    vec3 v = normalize(p);
    vec3 r = normalize(reflect(v, n));

    // March with growing steps (finer near the surface), jittered against banding
    int steps = clamp(ssrSteps, 4, MAX_STEPS);
    float jitter = ivfPixelNoise();
    vec3 start = p + n * (0.002 * -p.z);
    float prevT = 0.0;
    float hitT = -1.0;
    vec2 hitUV = vec2(0.0);
    for (int i = 1; i <= MAX_STEPS; ++i)
    {
        if (i > steps)
            break;
        float f = (float(i) - 1.0 + jitter) / float(steps);
        float t = ssrMaxDistance * f * f + 0.01;
        float w;
        vec2 uv = ivfProject(start + r * t, w);
        if (w <= 0.0 || uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            break;
        float sceneDepth = ivfDepthAt(uv);
        float rayZ = (start + r * t).z;
        float dz = ivfViewPosFromDepth(uv, sceneDepth).z - rayZ; // > 0: ray behind the surface
        if (sceneDepth < 1.0 && dz > 0.0 && dz < ssrThickness + (t - prevT))
        {
            // Refine between the last two steps
            float lo = prevT;
            float hi = t;
            for (int k = 0; k < 6; ++k)
            {
                float mid = 0.5 * (lo + hi);
                vec2 muv = ivfProject(start + r * mid, w);
                float mdz = ivfViewPosFromDepth(muv, ivfDepthAt(muv)).z - (start + r * mid).z;
                if (mdz > 0.0)
                    hi = mid;
                else
                    lo = mid;
            }
            hitT = hi;
            hitUV = ivfProject(start + r * hi, w);
            break;
        }
        prevT = t;
    }

    vec3 reflection = vec3(0.0);
    float amount = 0.0;
    if (hitT > 0.0)
    {
        vec2 edge = min(hitUV, 1.0 - hitUV);
        float edgeFade = clamp(min(edge.x, edge.y) / max(ssrEdgeFade, 1e-4), 0.0, 1.0);
        float distanceFade = 1.0 - clamp(hitT / ssrMaxDistance, 0.0, 1.0);
        float fresnel = pow(1.0 - max(dot(-v, n), 0.0), 5.0);
        amount = ssrStrength * mask * edgeFade * distanceFade * mix(1.0, fresnel, clamp(ssrFresnel, 0.0, 1.0));

        float maxLod = float(max(screenTextureLevels - 1, 0));
        float lod = min(ssrRoughness * 6.0 * (0.3 + 0.7 * clamp(hitT / (0.5 * ssrMaxDistance), 0.0, 1.0)), maxLod);
        reflection = textureLod(screenTexture, hitUV, lod).rgb;
    }

    FragColor = ssrShow ? vec4(reflection * amount, 1.0) : vec4(mix(color.rgb, reflection, amount), color.a);
}
)";

} // namespace ivf
