#pragma once

#include <string>

namespace ivf {

inline const std::string render_to_texture_vert_shader_source_330 = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}
)";

inline const std::string render_to_texture_frag_shader_source_330 = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
})";

inline const std::string vignette_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform float vignetteSize = 1.5;
uniform float vignetteSmoothness = 1.0;

// Vignette effect
vec3 applyVignette(vec3 color, vec2 texCoord) {
    vec2 position = (texCoord - 0.5) * 2.0;
    float len = length(position);

    float vignette = smoothstep(vignetteSize, vignetteSize - vignetteSmoothness, len);
    return color * vignette;
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // Apply vignette effect
    col = applyVignette(col, TexCoords);
    FragColor = vec4(col, 1.0);
})";

inline const std::string chromatic_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

// Chromatic aberration
vec3 applyChromaticAberration(sampler2D tex, vec2 texCoord) {
    float offset = 0.005;
    vec2 distort = (texCoord - 0.5) * offset;
    
    vec3 col;
    col.r = texture(tex, texCoord + distort).r;
    col.g = texture(tex, texCoord).g;
    col.b = texture(tex, texCoord - distort).b;
    return col;
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // Apply vignette effect
    col = applyChromaticAberration(screenTexture, TexCoords);
    FragColor = vec4(col, 1.0);
})";

inline const std::string filmgrain_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

// Utility functions
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

// Film grain
vec3 applyFilmGrain(vec3 color, vec2 texCoord, float time) {
    float noise = random(texCoord + time) * 0.1;
    return color + noise * 0.1;
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // Apply vignette effect
    col = applyFilmGrain(col, TexCoords, time);
    FragColor = vec4(col, 1.0);
})";

inline const std::string blur_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

uniform float blurRadius = 2.0;

// Gaussian blur
vec3 applyBlur(sampler2D tex, vec2 texCoord) {
    vec2 texelSize = 1.0 / textureSize(tex, 0);
    vec3 result = vec3(0.0);
    float total = 0.0;
    
    for(float x = -blurRadius; x <= blurRadius; x++) {
        for(float y = -blurRadius; y <= blurRadius; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            float weight = exp(-(x*x + y*y) / (2.0 * blurRadius * blurRadius));
            result += texture(tex, texCoord + offset).rgb * weight;
            total += weight;
        }
    }
    
    return result / total;
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // Apply vignette effect
    col = applyBlur(screenTexture, TexCoords);
    FragColor = vec4(col, 1.0);
})";

inline const std::string tint_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

// Color tinting (sepia example)
vec3 applyColorTint(vec3 color) {
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    vec3 sepiaColor = vec3(gray * 1.2, gray * 0.9, gray * 0.7);
    return mix(color, sepiaColor, 0.5);
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // Apply vignette effect
    col = applyColorTint(col);
    FragColor = vec4(col, 1.0);
})";

} // namespace ivf
