#pragma once

#include <string>

namespace ivf {

inline const std::string basic_vert_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

out vec3 fragPos;
out vec3 normal;
out vec4 color;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal = normalMatrix * aNormal;

    color = aColor;
    texCoord = aTex;
    
    gl_Position = projection * view * vec4(fragPos, 1.0);
}
)";

inline const std::string basic_frag_shader_source = R"(
#version 330 core

//
// A lot of information from:
//
// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/2.lighting/6.multiple_lights/6.multiple_lights.fs
//

// Blend mode enumeration
#define BLEND_NORMAL 0
#define BLEND_MULTIPLY 1
#define BLEND_ADD 2
#define BLEND_SCREEN 3
#define BLEND_OVERLAY 4
#define BLEND_DECAL 5

out vec4 fragColor;

struct Material 
{
    vec3 diffuseColor;
    vec3 specularColor;
    vec3 ambientColor;
    float shininess;
};

struct DirLight 
{
    bool enabled;
    vec3 direction;	

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct PointLight 
{
    bool enabled;
    vec3 position;    

    float constant;
    float linear;
    float quadratic;
	
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct SpotLight 
{
    bool enabled;
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;       
};

in vec3 normal;  
in vec3 fragPos;  
in vec4 color;
in vec2 texCoord;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec4 lightColor;
uniform vec3 textColor;

uniform bool useLighting;
uniform bool useTexture = false;
uniform bool useVertexColors;
uniform bool usePointFalloff = false;
uniform bool textRendering = false;
uniform bool useFixedTextColor = false;

uniform float point_falloff_a = 0.0;
uniform float point_falloff_b = 0.0;

uniform int pointLightCount = 0;
uniform int dirLightCount = 0;
uniform int spotLightCount = 0;

uniform sampler2D texture0;

#define NR_POINT_LIGHTS 8
#define NR_DIR_LIGHTS 8
#define NR_SPOT_LIGHTS 8

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

uniform Material material;

uniform int blendMode = BLEND_MULTIPLY;
uniform float blendFactor = 0.5; // Controls the strength of the blend [0,1]

uniform bool selectionRendering = false;
uniform uint objectId;

vec4 applyBlendMode(vec4 textureColor, vec4 baseColor);
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{	
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);   
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 result = vec3(0.0, 0.0, 0.0);

    for(int i = 0; i < pointLightCount; i++)
    {
        if (pointLights[i].enabled)
            result += calcPointLight(pointLights[i], norm, fragPos, viewDir);    
    }

    for(int i = 0; i < dirLightCount; i++)
    {
        if (dirLights[i].enabled)
            result += calcDirLight(dirLights[i], norm, viewDir);    
    }

    for(int i = 0; i < spotLightCount; i++)
    {
        if (spotLights[i].enabled)
            result += calcSpotLight(spotLights[i], norm, fragPos, viewDir);
    }

    if (selectionRendering) 
    {
        uint mask = uint(255);  // 0xFF
        uint r = objectId & mask;
        uint g = (objectId >> uint(8)) & mask;
        uint b = (objectId >> uint(16)) & mask;
    
        fragColor = vec4(float(r) / 255.0, 
                         float(g) / 255.0, 
                         float(b) / 255.0, 
                         1.0);
    }
    else
    {
        if (useLighting) 
        {
            vec4 baseColor = vec4(result, 1.0);
    
            if (useTexture) 
            {
                if (textRendering) 
                {
                    if (useFixedTextColor) 
                    {
                        vec4 texSample = vec4(textColor.rgb, texture(texture0, texCoord).r);
                        fragColor = applyBlendMode(texSample, baseColor);
                    }
                    else 
                    {
                        vec4 texSample = vec4(result.rgb, texture(texture0, texCoord).r);
                        fragColor = applyBlendMode(texSample, baseColor);
                    }
                } 
                else 
                {
                    vec4 texSample = texture(texture0, texCoord);
                    fragColor = applyBlendMode(texSample, baseColor);
                }
            } 
            else 
            {
                fragColor = baseColor;
            }
        }
        else
        {
            if (useVertexColors) 
            {
                fragColor = color;
            } 
            else 
            {
                fragColor = vec4(material.diffuseColor, 1.0);
            }
        }
    }
} 

// -----------------------------------------------------------------------------

// calculates the color when using a directional light.
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    // diffuse shading

    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // combine results

    vec3 ambient = light.ambientColor * material.ambientColor;
    vec3 diffuse = light.diffuseColor * diff * material.diffuseColor;
    vec3 specular = light.specularColor * spec * material.specularColor;
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
 
    // diffuse shading

    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // attenuation

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    // combine results

    vec3 ambient = light.ambientColor * material.ambientColor;
    vec3 diffuse = light.diffuseColor * diff * material.diffuseColor;
    vec3 specular = light.specularColor * spec * material.specularColor;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // diffuse shading

    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // attenuation

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    // spotlight intensity

    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // combine results

    vec3 ambient = light.ambientColor * material.ambientColor;
    vec3 diffuse = light.diffuseColor * diff * material.diffuseColor;
    vec3 specular = light.specularColor * spec * material.specularColor;

    //ambient = vec3(light.cutOff, light.outerCutOff, 0.0);

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

vec4 applyBlendMode(vec4 textureColor, vec4 baseColor) {
    vec4 result;
    
    switch(blendMode) {
        case BLEND_MULTIPLY:
            result = textureColor * baseColor;
            break;
            
        case BLEND_ADD:
            result = min(textureColor + baseColor, vec4(1.0));
            break;
            
        case BLEND_SCREEN:
            result = vec4(1.0) - (vec4(1.0) - textureColor) * (vec4(1.0) - baseColor);
            break;
            
        case BLEND_OVERLAY:
            result = vec4(
                baseColor.r < 0.5 ? (2.0 * baseColor.r * textureColor.r) : (1.0 - 2.0 * (1.0 - baseColor.r) * (1.0 - textureColor.r)),
                baseColor.g < 0.5 ? (2.0 * baseColor.g * textureColor.g) : (1.0 - 2.0 * (1.0 - baseColor.g) * (1.0 - textureColor.g)),
                baseColor.b < 0.5 ? (2.0 * baseColor.b * textureColor.b) : (1.0 - 2.0 * (1.0 - baseColor.b) * (1.0 - textureColor.b)),
                baseColor.a
            );
            break;
            
        case BLEND_DECAL:
            result = vec4(
                mix(baseColor.rgb, textureColor.rgb, textureColor.a),
                baseColor.a
            );
            break;
            
        case BLEND_NORMAL:
        default:
            result = textureColor;
            break;
    }
    
    return mix(baseColor, result, blendFactor);
}
)";

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

// Vignette effect
vec3 applyVignette(vec3 color, vec2 texCoord) {
    vec2 position = (texCoord - 0.5) * 2.0;
    float len = length(position);
    float vignette = smoothstep(1.0, 0.4, len);
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

// Gaussian blur
vec3 applyBlur(sampler2D tex, vec2 texCoord) {
    vec2 texelSize = 1.0 / textureSize(tex, 0);
    vec3 result = vec3(0.0);
    float total = 0.0;
    float blurRadius = 2.0;
    
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

}; // namespace ivf
