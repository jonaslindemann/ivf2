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
uniform float vignetteSize;
uniform float vignetteSmoothness;

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
uniform float offset;

// Chromatic aberration
vec3 applyChromaticAberration(sampler2D tex, vec2 texCoord) {
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
uniform float noiseIntensity;
uniform float grainBlending;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

vec3 applyFilmGrain(vec3 color, vec2 texCoord, float time) {
    float noise = random(texCoord + time) * noiseIntensity;
    return color + noise * grainBlending;
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    col = applyFilmGrain(col, TexCoords, time);
    FragColor = vec4(col, 1.0);
})";

inline const std::string blur_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

uniform float blurRadius;

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
uniform vec3 tintColor;
uniform float tintStrength;
uniform vec3 grayScaleWeights;

vec3 applyColorTint(vec3 color) {
    float gray = dot(color, grayScaleWeights);
    vec3 tintedColor = vec3(gray) * tintColor;
    return mix(color, tintedColor, tintStrength);
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    col = applyColorTint(col);
    FragColor = vec4(col, 1.0);
})";

inline const std::string bloom_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform float threshold;
uniform float intensity;

vec3 applyBloom(sampler2D tex, vec2 texCoord) {
    vec3 color = texture(tex, texCoord).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    vec3 brightPass = color * step(threshold, brightness);
    return color + brightPass * intensity;
}

void main()
{
    vec3 col = applyBloom(screenTexture, TexCoords);
    FragColor = vec4(col, 1.0);
})";

inline const std::string dither_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform float time;

uniform sampler2D screenTexture;
uniform float ditherMatrix[16] = float[](
    0.0/16.0, 8.0/16.0, 2.0/16.0, 10.0/16.0,
    12.0/16.0, 4.0/16.0, 14.0/16.0, 6.0/16.0,
    3.0/16.0, 11.0/16.0, 1.0/16.0, 9.0/16.0,
    15.0/16.0, 7.0/16.0, 13.0/16.0, 5.0/16.0
);

vec3 applyDithering(vec3 color, vec2 pos) {
    int x = int(mod(gl_FragCoord.x, 4.0));
    int y = int(mod(gl_FragCoord.y, 4.0));
    float threshold = ditherMatrix[y * 4 + x];
    return step(threshold, color);
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    col = applyDithering(col, gl_FragCoord.xy);
    FragColor = vec4(col, 1.0);
})";

inline const std::string pixelate_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float pixelSize;
uniform float time;


vec3 applyPixelation(sampler2D tex, vec2 texCoord) {
    vec2 texSize = textureSize(tex, 0);
    vec2 pixel = floor(texCoord * texSize / pixelSize) * pixelSize / texSize;
    return texture(tex, pixel).rgb;
}

void main()
{
    vec3 col = applyPixelation(screenTexture, TexCoords);
    FragColor = vec4(col, 1.0);
})";

inline const std::string edge_detect_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float sensitivity;
uniform float time;


vec3 applyEdgeDetection(sampler2D tex, vec2 texCoord) {
    vec2 texelSize = 1.0 / textureSize(tex, 0);
    vec3 center = texture(tex, texCoord).rgb;
    vec3 left = texture(tex, texCoord - vec2(texelSize.x, 0.0)).rgb;
    vec3 right = texture(tex, texCoord + vec2(texelSize.x, 0.0)).rgb;
    vec3 up = texture(tex, texCoord - vec2(0.0, texelSize.y)).rgb;
    vec3 down = texture(tex, texCoord + vec2(0.0, texelSize.y)).rgb;
    
    vec3 dx = right - left;
    vec3 dy = down - up;
    vec3 edge = sqrt(dx * dx + dy * dy) * sensitivity;
    return edge;
}

void main()
{
    vec3 col = applyEdgeDetection(screenTexture, TexCoords);
    FragColor = vec4(col, 1.0);
})";

} // namespace ivf
