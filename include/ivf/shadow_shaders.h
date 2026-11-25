#pragma once

#include <string>

namespace ivf {

/**
 * @file shadow_shaders.h
 * @brief GLSL for shadow mapping: depth pass shaders and the shadow sampling functions.
 *
 * shadow_glsl is pasted into fragment shaders after IVF_CAMERA_BLOCK_GLSL and
 * IVF_LIGHTS_BLOCK_GLSL (it reads `view` and the `shadows` array). It declares
 * `uniform sampler2D shadowMaps[IVF_MAX_SHADOW_MAPS]` (Program::link() points them at
 * texture units ShadowMapUnit + i) and
 *
 * @code{.glsl}
 * // 0 = lit, 1 = fully shadowed. slot: the light's shadowIndex (-1: no shadow).
 * float ivfShadowFactor(int slot, vec3 worldPos, vec3 normal, vec3 toLight);
 * // Tile (cascade or cube face) used for worldPos, -1 outside the shadow.
 * int ivfShadowTileIndex(int slot, vec3 worldPos);
 * @endcode
 *
 * @code{.cpp}
 * const std::string frag = std::string(R"(#version 330 core
 * ...)" IVF_CAMERA_BLOCK_GLSL IVF_LIGHTS_BLOCK_GLSL) + ivf::shadow_glsl + R"(
 * ...
 *     float shadow = ivfShadowFactor(dirLights[i].shadowIndex, fragPos, n, -dirLights[i].direction);
 *     color += (1.0 - shadow * dirLights[i].shadowStrength) * direct;
 * )";
 * @endcode
 *
 * A shadow map is an atlas of tiles in one depth texture: the cascades of a directional
 * light (chosen by view depth) or the six faces of a point light (chosen by the major axis
 * of the direction from the light). Filters: hard (one sample), PCF (a square of
 * (2r + 1)^2 samples) and PCSS (blocker search, then a filter as wide as the penumbra).
 */
inline const std::string shadow_glsl = R"(
uniform sampler2D shadowMaps[IVF_MAX_SHADOW_MAPS];

const vec2 ivfPoisson[16] = vec2[](
    vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725), vec2(-0.09418410, -0.92938870),
    vec2(0.34495938, 0.29387760), vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379), vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420), vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590), vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790));

// Tile (cascade or cube face) for a world position, -1 outside the shadow
int ivfShadowTileIndex(int slot, vec3 worldPos)
{
    if (slot < 0)
        return -1;
    int type = shadows[slot].info.x;
    int count = shadows[slot].info.y;
    if (type == 2)
    {
        vec3 v = worldPos - shadows[slot].lightPos.xyz;
        vec3 a = abs(v);
        if (a.x >= a.y && a.x >= a.z)
            return v.x > 0.0 ? 0 : 1;
        if (a.y >= a.z)
            return v.y > 0.0 ? 2 : 3;
        return v.z > 0.0 ? 4 : 5;
    }
    if (type != 1)
        return -1;
    if (count <= 1)
        return 0;
    float depth = -(view * vec4(worldPos, 1.0)).z;
    for (int k = 0; k < count; ++k)
        if (depth < shadows[slot].splits[k])
            return k;
    return -1;
}

// Depth comparable with the receiver's: the stored depth (directional) or the linear
// distance along the face axis (point)
float ivfShadowDepth(sampler2D map, vec2 uv, int type, vec2 nearFar)
{
    float d = texture(map, uv).r;
    if (type != 2)
        return d;
    float z = d * 2.0 - 1.0;
    return 2.0 * nearFar.x * nearFar.y / (nearFar.y + nearFar.x - z * (nearFar.y - nearFar.x));
}

float ivfShadow(sampler2D map, int slot, vec3 worldPos, vec3 N, vec3 L)
{
    int tile = ivfShadowTileIndex(slot, worldPos);
    if (tile < 0)
        return 0.0;

    int type = shadows[slot].info.x;
    int filterMode = shadows[slot].info.z;
    int radius = shadows[slot].info.w;
    vec4 params = shadows[slot].params; // bias, normal offset (texels), light size
    vec4 atlas = shadows[slot].atlas;   // tile size, columns, rows, tan(fov / 2)
    vec2 nearFar = shadows[slot].splits.xy;
    float res = atlas.x;

    // World size of a shadow map texel at the receiver
    float texelWorld;
    if (type == 2)
    {
        vec3 a = abs(worldPos - shadows[slot].lightPos.xyz);
        texelWorld = 2.0 * max(a.x, max(a.y, a.z)) * atlas.w / res;
    }
    else
    {
        mat4 M0 = shadows[slot].matrices[tile];
        texelWorld = 2.0 / (length(vec3(M0[0][0], M0[1][0], M0[2][0])) * res);
    }

    // Normal offset against acne on surfaces at grazing angles to the light
    vec3 pos = worldPos + N * (params.y * texelWorld);
    if (type == 2 && params.y != 0.0)
        tile = ivfShadowTileIndex(slot, pos);

    mat4 M = shadows[slot].matrices[tile];
    vec4 p = M * vec4(pos, 1.0);
    vec3 proj = p.xyz / p.w * 0.5 + 0.5;
    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z < 0.0 || proj.z > 1.0)
        return 0.0;

    // Tile in the atlas; samples stay inside it
    float cols = atlas.y;
    float rows = atlas.z;
    vec2 tileOrigin = vec2(mod(float(tile), cols), floor(float(tile) / cols)) / vec2(cols, rows);
    vec2 tileSize = vec2(1.0 / cols, 1.0 / rows);
    vec2 texel = 1.0 / vec2(textureSize(map, 0));
    vec2 lo = tileOrigin + 0.5 * texel;
    vec2 hi = tileOrigin + tileSize - 0.5 * texel;
    vec2 uv = tileOrigin + proj.xy * tileSize;

    float b = max(2.0 * params.x * (1.0 - dot(N, L)), params.x);
    float current = proj.z;
    float bias = b;
    if (type == 2)
    {
        vec3 a = abs(pos - shadows[slot].lightPos.xyz);
        current = max(a.x, max(a.y, a.z));
        bias = b * nearFar.y;
    }

    // Samples away from the centre see the receiving surface itself nearer to the light
    // when it is tilted; wide kernels (PCSS, point lights) raise the bias with the offset
    float worldPerDepth = 1.0;
    if (type != 2)
        worldPerDepth = 2.0 / length(vec3(M[0][2], M[1][2], M[2][2]));
    float ndl = clamp(abs(dot(N, L)), 0.1, 1.0);
    float slope = 0.0;
    if (filterMode == 2 || type == 2)
        slope = sqrt(1.0 - ndl * ndl) / ndl * texelWorld / worldPerDepth;

    if (filterMode == 0)
        return current - bias > ivfShadowDepth(map, clamp(uv, lo, hi), type, nearFar) ? 1.0 : 0.0;

    if (filterMode == 1)
    {
        float shadow = 0.0;
        float n = 0.0;
        for (int x = -radius; x <= radius; ++x)
        {
            for (int y = -radius; y <= radius; ++y)
            {
                float d = ivfShadowDepth(map, clamp(uv + vec2(x, y) * texel, lo, hi), type, nearFar);
                shadow += current - bias - slope * length(vec2(x, y)) > d ? 1.0 : 0.0;
                n += 1.0;
            }
        }
        return shadow / n;
    }

    // PCSS: average depth of the blockers, then a filter as wide as the penumbra
    float angle = 6.2831853 * fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
    mat2 rotation = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
    float lightSize = params.z; // directional: tan of the light's angular radius; point: radius

    float searchWorld = type == 2 ? lightSize : lightSize * current * worldPerDepth;
    float searchTexels = clamp(searchWorld / texelWorld, 1.0, 24.0);
    float blockerSum = 0.0;
    float blockers = 0.0;
    for (int i = 0; i < 16; ++i)
    {
        vec2 offset = rotation * ivfPoisson[i] * searchTexels;
        float d = ivfShadowDepth(map, clamp(uv + offset * texel, lo, hi), type, nearFar);
        if (d < current - bias - slope * length(offset))
        {
            blockerSum += d;
            blockers += 1.0;
        }
    }
    if (blockers == 0.0)
        return 0.0;

    float blocker = blockerSum / blockers;
    float penumbraWorld = type == 2 ? (current - blocker) / max(blocker, 1e-4) * lightSize
                                    : (current - blocker) * worldPerDepth * lightSize;
    float filterTexels = clamp(penumbraWorld / texelWorld, 1.0, 24.0);
    float shadow = 0.0;
    for (int i = 0; i < 16; ++i)
    {
        vec2 offset = rotation * ivfPoisson[i] * filterTexels;
        float d = ivfShadowDepth(map, clamp(uv + offset * texel, lo, hi), type, nearFar);
        shadow += current - bias - slope * length(offset) > d ? 1.0 : 0.0;
    }
    return shadow / 16.0;
}

// 0 = lit, 1 = shadowed. Sampler arrays need constant indices in GLSL 3.30, hence the branches.
float ivfShadowFactor(int slot, vec3 worldPos, vec3 N, vec3 toLight)
{
    if (slot == 0)
        return ivfShadow(shadowMaps[0], 0, worldPos, N, toLight);
    if (slot == 1)
        return ivfShadow(shadowMaps[1], 1, worldPos, N, toLight);
    if (slot == 2)
        return ivfShadow(shadowMaps[2], 2, worldPos, N, toLight);
    if (slot == 3)
        return ivfShadow(shadowMaps[3], 3, worldPos, N, toLight);
    return 0.0;
}
)";

/// Vertex shader for shadow map depth rendering (OpenGL 3.3+).
inline const std::string shadow_depth_vert_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";

/// Fragment shader for shadow map depth rendering (OpenGL 3.3+).
inline const std::string shadow_depth_frag_shader = R"(
#version 330 core

void main()
{
    // Depth is automatically written to the depth buffer
    // No need for color output since we only care about depth
}
)";

} // namespace ivf
