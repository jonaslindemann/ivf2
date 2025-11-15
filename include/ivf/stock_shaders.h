#pragma once

#include <string>

namespace ivf {

/**
 * @file stock_shaders.h
 * @brief Contains GLSL shader source code strings for basic rendering and lighting.
 *
 * This header provides inline string constants for vertex and fragment shaders used in
 * basic rendering, including support for lighting, shadow mapping, and texture blending.
 * These shaders are intended for use with OpenGL 3.3+ and can be loaded at runtime for
 * custom rendering pipelines.
 */

/// Vertex shader for basic rendering with lighting and shadow mapping (OpenGL 3.3+).
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

uniform bool shadowPass = false;
uniform mat4 lightSpaceMatrix;

void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    // Transform normal to world space
    // and then to view space
    //normal = normalMatrix * aNormal;
    normal = mat3(model) * aNormal; 

    color = aColor;
    texCoord = aTex;
    
    if (shadowPass) {
        // When rendering shadow map, just output position in light space
        gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    } else {
        // Normal rendering path
        gl_Position = projection * view * vec4(fragPos, 1.0);
    }
}
)";

/// Fragment shader for basic rendering with lighting, shadow mapping, and texture blending (OpenGL 4.0+).
inline const std::string basic_frag_shader_source = R"(
#version 400 core

//
// A lot of information from:
//
// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/2.lighting/6.multiple_lights/6.multiple_lights.fs
//

// Blend mode enumeration
#define TEX_BLEND_NORMAL 0
#define TEX_BLEND_MULTIPLY 1
#define TEX_BLEND_ADD 2
#define TEX_BLEND_SCREEN 3
#define TEX_BLEND_OVERLAY 4
#define TEX_BLEND_DECAL 5

// Multitexturing support
#define MAX_TEXTURES 8

out vec4 fragColor;

struct Material 
{
    vec3 diffuseColor;
    vec3 specularColor;
    vec3 ambientColor;
    float alpha;
    float shininess;
};

struct DirLight 
{
    bool enabled;

    vec3 direction;	

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    bool castShadows;
    float shadowStrength;
    mat4 lightSpaceMatrix;
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
    float shadowStrength;
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
    float shadowStrength;
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

// Multitexturing uniforms
uniform bool useMultiTexturing = false;
uniform int activeTextureCount = 1;
uniform sampler2D textures[MAX_TEXTURES];
uniform int textureBlendModes[MAX_TEXTURES];
uniform float textureBlendFactors[MAX_TEXTURES];

#define NR_POINT_LIGHTS 8
#define NR_DIR_LIGHTS 8
#define NR_SPOT_LIGHTS 8

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

uniform Material material;

uniform int blendMode = TEX_BLEND_MULTIPLY;
uniform float blendFactor = 0.5;

uniform bool selectionRendering = false;
uniform uint objectId;

uniform bool shadowPass = false;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform bool useShadows = false;
uniform int debugShadow = 0;

uniform sampler2D shadowMaps[NR_DIR_LIGHTS];
uniform mat4 lightSpaceMatrices[NR_DIR_LIGHTS];

vec4 applyTexBlendMode(vec4 textureColor, vec4 baseColor);
vec4 applyTexBlendModeIndexed(vec4 textureColor, vec4 baseColor, int mode, float factor);
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, mat4 lightMat, sampler2D sMap);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float calculateShadow(vec4 fragPosLightSpace, sampler2D sMap);

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{	
    if (shadowPass) 
    {
        return;
    }

    if (debugShadow > 0) 
    {
        vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
    
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        float currentDepth = projCoords.z;

        if (debugShadow == 1) 
        {
            fragColor = vec4(vec3(closestDepth), 1.0);
            return;
        } 
        else if (debugShadow == 2) 
        {
            float shadow = currentDepth - 0.05 > closestDepth ? 1.0 : 0.0;
            fragColor = vec4(shadow, 1.0-shadow, 0.0, 1.0);
            return;
        } 
        else if (debugShadow == 3) 
        {
            fragColor = vec4(projCoords, 1.0);
            return;
        }
        else if (debugShadow == 4) 
        {
            float closestDepth = texture(shadowMap, projCoords.xy).r;
            float currentDepth = projCoords.z;
            fragColor = vec4(currentDepth, closestDepth, 0.0, 1.0);
            return;
        }
    }

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
            result += calcDirLight(dirLights[i], norm, viewDir, lightSpaceMatrices[i], shadowMaps[i]);    
    }

    for(int i = 0; i < spotLightCount; i++)
    {
        if (spotLights[i].enabled)
            result += calcSpotLight(spotLights[i], norm, fragPos, viewDir);
    }

    if (selectionRendering) 
    {
        uint mask = uint(255);
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
                        fragColor = texSample;
                    }
                    else 
                    {
                        vec4 texSample = vec4(result.rgb, texture(texture0, texCoord).r);
                        fragColor = texSample;
                    }
                } 
                else 
                {
                    vec4 finalTexColor;
                    
                    if (useMultiTexturing && activeTextureCount > 1)
                    {
                        finalTexColor = texture(textures[0], texCoord);
                        
                        for(int i = 1; i < activeTextureCount && i < MAX_TEXTURES; i++)
                        {
                            vec4 texSample = texture(textures[i], texCoord);
                            finalTexColor = applyTexBlendModeIndexed(
                                texSample, 
                                finalTexColor, 
                                textureBlendModes[i],
                                textureBlendFactors[i]
                            );
                        }
                    }
                    else
                    {
                        finalTexColor = texture(texture0, texCoord);
                    }
                    
                    fragColor = applyTexBlendMode(finalTexColor, baseColor);
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
                fragColor = vec4(material.diffuseColor, material.alpha);
            }
        }
    }
} 

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, mat4 lightMat, sampler2D sMap)
{
    vec3 norm = normalize(normal);
    
    norm = dot(norm, viewDir) < -0.1 ? -norm : norm;
    
    vec3 lightDir = normalize(-light.direction);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 ambient = light.ambientColor * material.ambientColor;
    vec3 diffuse = light.diffuseColor * diff * material.diffuseColor;
    vec3 specular = light.specularColor * spec * material.specularColor;
    float shadow = 0.0;
    if(useShadows && light.castShadows)
    {
        vec4 fragPosLightSpace = lightMat * vec4(fragPos, 1.0);
        shadow = calculateShadow(fragPosLightSpace, sMap);
    }
    
    return (ambient + (1.0 - shadow * light.shadowStrength) * (diffuse + specular));
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 norm = normalize(normal);
    
    norm = dot(norm, viewDir) < -0.1 ? -norm : norm;
    
    vec3 lightDir = normalize(light.position - fragPos);
 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    vec3 ambient = light.ambientColor * material.ambientColor;
    vec3 diffuse = light.diffuseColor * diff * material.diffuseColor;
    vec3 specular = light.specularColor * spec * material.specularColor;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 norm = normalize(normal);
    
    norm = dot(norm, viewDir) < -0.1 ? -norm : norm;
    
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 ambient = light.ambientColor * material.ambientColor;
    vec3 diffuse = light.diffuseColor * diff * material.diffuseColor;
    vec3 specular = light.specularColor * spec * material.specularColor;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

vec4 applyTexBlendMode(vec4 textureColor, vec4 baseColor) 
{
    vec4 result;
    
    switch(blendMode) {
        case TEX_BLEND_MULTIPLY:
            result = textureColor * baseColor;
            break;
            
        case TEX_BLEND_ADD:
            result = min(textureColor + baseColor, vec4(1.0));
            break;
            
        case TEX_BLEND_SCREEN:
            result = vec4(1.0) - (vec4(1.0) - textureColor) * (vec4(1.0) - baseColor);
            break;
            
        case TEX_BLEND_OVERLAY:
            result = vec4(
                baseColor.r < 0.5 ? (2.0 * baseColor.r * textureColor.r) : (1.0 - 2.0 * (1.0 - baseColor.r) * (1.0 - textureColor.r)),
                baseColor.g < 0.5 ? (2.0 * baseColor.g * textureColor.g) : (1.0 - 2.0 * (1.0 - baseColor.g) * (1.0 - textureColor.g)),
                baseColor.b < 0.5 ? (2.0 * baseColor.b * textureColor.b) : (1.0 - 2.0 * (1.0 - baseColor.b) * (1.0 - textureColor.b)),
                baseColor.a
            );
            break;
            
        case TEX_BLEND_DECAL:
            result = vec4(
                mix(baseColor.rgb, textureColor.rgb, textureColor.a),
                baseColor.a
            );
            break;
            
        case TEX_BLEND_NORMAL:
        default:
            result = textureColor;
            break;
    }
    
    return mix(baseColor, result, blendFactor);
}

vec4 applyTexBlendModeIndexed(vec4 textureColor, vec4 baseColor, int mode, float factor) 
{
    vec4 result;
    
    switch(mode) {
        case TEX_BLEND_MULTIPLY:
            result = textureColor * baseColor;
            break;
            
        case TEX_BLEND_ADD:
            result = min(textureColor + baseColor, vec4(1.0));
            break;
            
        case TEX_BLEND_SCREEN:
            result = vec4(1.0) - (vec4(1.0) - textureColor) * (vec4(1.0) - baseColor);
            break;
            
        case TEX_BLEND_OVERLAY:
            result = vec4(
                baseColor.r < 0.5 ? (2.0 * baseColor.r * textureColor.r) : (1.0 - 2.0 * (1.0 - baseColor.r) * (1.0 - textureColor.r)),
                baseColor.g < 0.5 ? (2.0 * baseColor.g * textureColor.g) : (1.0 - 2.0 * (1.0 - baseColor.g) * (1.0 - textureColor.g)),
                baseColor.b < 0.5 ? (2.0 * baseColor.b * textureColor.b) : (1.0 - 2.0 * (1.0 - baseColor.b) * (1.0 - textureColor.b)),
                baseColor.a
            );
            break;
            
        case TEX_BLEND_DECAL:
            result = vec4(
                mix(baseColor.rgb, textureColor.rgb, textureColor.a),
                baseColor.a
            );
            break;
            
        case TEX_BLEND_NORMAL:
        default:
            result = textureColor;
            break;
    }
    
    return mix(baseColor, result, factor);
}

float calculateShadow(vec4 fragPosLightSpace, sampler2D sMap)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0 ||
       projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }
    
    float closestDepth = texture(sMap, projCoords.xy).r; 
    
    float currentDepth = projCoords.z;
    
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(-dirLights[0].direction);

    float bias = max(0.002 * (1.0 - dot(norm, lightDir)), 0.001);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(sMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(sMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }

    shadow /= 9.0;
    
    return shadow;
}

)";

}; // namespace ivf
