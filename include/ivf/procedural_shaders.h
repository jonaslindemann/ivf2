#pragma once

/**
 * @file procedural_shaders.h
 * @brief GLSL shader source code for GPU-based procedural texture generation.
 */

#include <string>

namespace ivf {

/**
 * @brief Vertex shader for fullscreen quad rendering (used by all procedural shaders).
 */
inline const std::string proc_quad_vert_shader = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTex;

out vec2 texCoord;

void main() {
    texCoord = aTex;
    gl_Position = vec4(aPos, 1.0);
}
)";

/**
 * @brief Fragment shader for checkerboard pattern generation.
 */
inline const std::string proc_checkerboard_frag_shader = R"(
#version 330 core

in vec2 texCoord;
out vec4 fragColor;

uniform vec4 color1 = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4 color2 = vec4(0.0, 0.0, 0.0, 1.0);
uniform float checkerSize = 8.0;
uniform float time = 0.0;

void main() {
    // Animate the checkerboard by offsetting based on time
    vec2 animatedCoord = texCoord + vec2(time * 0.1, 0.0);
    vec2 pos = floor(animatedCoord * checkerSize);
    float pattern = mod(pos.x + pos.y, 2.0);
    fragColor = mix(color1, color2, pattern);
}
)";

/**
 * @brief Fragment shader for Perlin noise generation.
 */
inline const std::string proc_noise_frag_shader = R"(
#version 330 core

in vec2 texCoord;
out vec4 fragColor;

uniform vec4 colorLow = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4 colorHigh = vec4(1.0, 1.0, 1.0, 1.0);
uniform float scale = 4.0;
uniform int octaves = 4;
uniform float persistence = 0.5;
uniform float lacunarity = 2.0;
uniform float time = 0.0;

// Hash function for noise
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// 2D noise
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractional Brownian Motion
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for(int i = 0; i < octaves; i++) {
        value += noise(p * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return value / maxValue;
}

void main() {
    // Animate noise by scrolling over time
    vec2 animatedCoord = texCoord + vec2(time * 0.05, time * 0.03);
    float n = fbm(animatedCoord * scale);
    fragColor = mix(colorLow, colorHigh, n);
}
)";

/**
 * @brief Fragment shader for gradient generation.
 */
inline const std::string proc_gradient_frag_shader = R"(
#version 330 core

in vec2 texCoord;
out vec4 fragColor;

uniform vec4 colorStart = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4 colorEnd = vec4(1.0, 1.0, 1.0, 1.0);
uniform int gradientType = 0; // 0=linear, 1=radial
uniform float angle = 0.0;
uniform vec2 center = vec2(0.5, 0.5);
uniform float time = 0.0;

void main() {
    float t;
    
    // Animate angle for linear gradients
    float animatedAngle = angle + time * 0.5;
    
    if (gradientType == 0) {
        vec2 dir = vec2(cos(animatedAngle), sin(animatedAngle));
        t = dot(texCoord, dir);
        t = clamp(t, 0.0, 1.0);
    } else {
        // Animate center for radial gradients
        vec2 animatedCenter = center + vec2(sin(time * 0.3) * 0.1, cos(time * 0.4) * 0.1);
        vec2 delta = texCoord - animatedCenter;
        t = length(delta);
        t = clamp(t, 0.0, 1.0);
    }
    
    fragColor = mix(colorStart, colorEnd, t);
}
)";

} // namespace ivf
