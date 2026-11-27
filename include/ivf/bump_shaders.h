#pragma once

#include <ivf/shadow_shaders.h>
#include <ivf/uniform_blocks.h>
#include <string>

namespace ivf {

/// Vertex shader for bump-mapped Phong rendering (OpenGL 4.0+).
/// Attribute layout is identical to the basic vertex shader so existing Mesh VAOs
/// work unchanged — no tangent vertex attribute is needed because TBN is rebuilt
/// in the fragment shader from screen-space derivatives.
inline const std::string bump_vert_shader_source = R"(
#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

out vec3 fragPos;
out vec3 normal;
out vec4 color;
out vec2 texCoord;

uniform mat4 model;
)" IVF_CAMERA_BLOCK_GLSL R"(
uniform bool shadowPass = false;
uniform mat4 lightSpaceMatrix;

void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(model) * aNormal;
    color = aColor;
    texCoord = aTex;

    if (shadowPass) {
        gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    } else {
        gl_Position = projection * view * vec4(fragPos, 1.0);
    }
}
)";

/// Fragment shader for bump-mapped Phong rendering (OpenGL 4.0+).
/// Extends the basic fragment shader with derivative-based TBN reconstruction
/// and normal-map sampling. When useNormalMap is false the shader is
/// functionally identical to the basic fragment shader.
inline const std::string bump_frag_shader_source = std::string(R"(
#version 400 core

#define TEX_BLEND_NORMAL   0
#define TEX_BLEND_MULTIPLY 1
#define TEX_BLEND_ADD      2
#define TEX_BLEND_SCREEN   3
#define TEX_BLEND_OVERLAY  4
#define TEX_BLEND_DECAL    5

#define MAX_TEXTURES 8

out vec4 fragColor;

struct Material
{
    vec3  diffuseColor;
    vec3  specularColor;
    vec3  ambientColor;
    float alpha;
    float shininess;
};

in vec3 normal;
in vec3 fragPos;
in vec4 color;
in vec2 texCoord;

uniform vec3 lightPos;
)" IVF_CAMERA_BLOCK_GLSL R"(
uniform vec4 lightColor;
uniform vec3 textColor;

uniform bool useLighting;
uniform bool useTexture       = false;
uniform bool useVertexColors;
uniform bool usePointFalloff  = false;
uniform bool textRendering    = false;
uniform bool useFixedTextColor = false;

uniform float point_falloff_a = 0.0;
uniform float point_falloff_b = 0.0;


uniform sampler2D texture0;

// Normal map
uniform bool      useNormalMap = false;
uniform sampler2D normalMap;

// Multitexturing
uniform bool useMultiTexturing    = false;
uniform int  activeTextureCount   = 1;
uniform sampler2D textures[MAX_TEXTURES];
uniform int   textureBlendModes[MAX_TEXTURES];
uniform float textureBlendFactors[MAX_TEXTURES];

// Sampler arrays may only be indexed with constants in GLSL ES (WebGL) and GLSL 3.30. Called
// in a loop: explicit derivatives (taken outside it) keep compilers from unrolling the loop.
vec4 sampleTextureUnit(int i, vec2 uv, vec2 dx, vec2 dy)
{
    switch (i)
    {
    case 1: return textureGrad(textures[1], uv, dx, dy);
    case 2: return textureGrad(textures[2], uv, dx, dy);
    case 3: return textureGrad(textures[3], uv, dx, dy);
    case 4: return textureGrad(textures[4], uv, dx, dy);
    case 5: return textureGrad(textures[5], uv, dx, dy);
    case 6: return textureGrad(textures[6], uv, dx, dy);
    case 7: return textureGrad(textures[7], uv, dx, dy);
    }
    return textureGrad(textures[0], uv, dx, dy);
}

)" IVF_LIGHTS_BLOCK_GLSL R"(

uniform Material material;

uniform int   blendMode   = TEX_BLEND_MULTIPLY;
uniform float blendFactor = 0.5;

uniform bool selectionRendering = false;
uniform uint objectId;

uniform bool      shadowPass  = false;
uniform mat4      lightSpaceMatrix;
uniform int       debugShadow = 0;
)") + shadow_glsl + R"(

uniform samplerCube envMap;
uniform bool        useEnvMap       = false;
uniform float       envReflectivity = 0.0;

vec4 applyTexBlendMode(vec4 textureColor, vec4 baseColor);
vec4 applyTexBlendModeIndexed(vec4 textureColor, vec4 baseColor, int mode, float factor);
vec3 calcDirLight(DirLight light, vec3 norm, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir);

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// Derivative-based TBN — no tangent vertex attribute required.
mat3 buildTBN(vec3 N)
{
    vec3 dp1  = dFdx(fragPos);
    vec3 dp2  = dFdy(fragPos);
    vec2 duv1 = dFdx(texCoord);
    vec2 duv2 = dFdy(texCoord);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}

void main()
{
    if (shadowPass)
    {
        return;
    }

    if (debugShadow > 0)
    {
        vec4  fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
        vec3  projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;

        float closestDepth = texture(shadowMaps[0], projCoords.xy).r;
        float currentDepth = projCoords.z;

        if (debugShadow == 1) {
            fragColor = vec4(vec3(closestDepth), 1.0);
            return;
        } else if (debugShadow == 2) {
            float shadow = currentDepth - 0.05 > closestDepth ? 1.0 : 0.0;
            fragColor = vec4(shadow, 1.0 - shadow, 0.0, 1.0);
            return;
        } else if (debugShadow == 3) {
            fragColor = vec4(projCoords, 1.0);
            return;
        } else if (debugShadow == 4) {
            fragColor = vec4(currentDepth, closestDepth, 0.0, 1.0);
            return;
        }
    }

    // Resolve surface normal — use normal map when available.
    vec3 norm = normalize(normal);
    if (useNormalMap)
    {
        mat3 TBN   = buildTBN(norm);
        vec3 tnorm = texture(normalMap, texCoord).rgb * 2.0 - 1.0;
        norm = normalize(TBN * tnorm);
    }

    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir  = normalize(viewPos  - fragPos);

    vec3 result = vec3(0.0);

    for (int i = 0; i < pointLightCount; i++)
        if (pointLights[i].enabled)
            result += calcPointLight(pointLights[i], norm, fragPos, viewDir);

    for (int i = 0; i < dirLightCount; i++)
        if (dirLights[i].enabled)
            result += calcDirLight(dirLights[i], norm, viewDir);

    for (int i = 0; i < spotLightCount; i++)
        if (spotLights[i].enabled)
            result += calcSpotLight(spotLights[i], norm, fragPos, viewDir);

    if (selectionRendering)
    {
        uint mask = uint(255);
        uint r = objectId & mask;
        uint g = (objectId >> uint(8))  & mask;
        uint b = (objectId >> uint(16)) & mask;
        fragColor = vec4(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0, 1.0);
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
                        fragColor = vec4(textColor.rgb, texture(texture0, texCoord).r);
                    else
                        fragColor = vec4(result.rgb,   texture(texture0, texCoord).r);
                }
                else
                {
                    vec4 finalTexColor;

                    if (useMultiTexturing && activeTextureCount > 1)
                    {
                        vec2 uvDx = dFdx(texCoord);
                        vec2 uvDy = dFdy(texCoord);
                        finalTexColor = texture(textures[0], texCoord);
                        for (int i = 1; i < activeTextureCount && i < MAX_TEXTURES; i++)
                        {
                            vec4 texSample = sampleTextureUnit(i, texCoord, uvDx, uvDy);
                            finalTexColor  = applyTexBlendModeIndexed(
                                texSample, finalTexColor,
                                textureBlendModes[i], textureBlendFactors[i]);
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
                fragColor = color;
            else
                fragColor = vec4(material.diffuseColor, material.alpha);
        }

        if (useEnvMap && envReflectivity > 0.0)
        {
            vec3 I = normalize(fragPos - viewPos);
            vec3 R = reflect(I, norm);
            fragColor.rgb = mix(fragColor.rgb, texture(envMap, R).rgb, envReflectivity);
        }
    }
}

vec3 calcDirLight(DirLight light, vec3 norm, vec3 viewDir)
{
    vec3 n = normalize(norm);
    n = dot(n, viewDir) < -0.1 ? -n : n;

    vec3  lightDir   = normalize(-light.direction);
    float diff       = max(dot(n, lightDir), 0.0);
    vec3  reflectDir = reflect(-lightDir, n);
    float spec       = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient  = light.ambientColor  * material.ambientColor;
    vec3 diffuse  = light.diffuseColor  * diff * material.diffuseColor;
    vec3 specular = light.specularColor * spec * material.specularColor;

    float shadow = ivfShadowFactor(light.shadowIndex, fragPos, normalize(normal), lightDir);

    return ambient + (1.0 - shadow * light.shadowStrength) * (diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir)
{
    vec3 n = normalize(norm);
    n = dot(n, viewDir) < -0.1 ? -n : n;

    vec3  lightDir   = normalize(light.position - fragPos);
    float diff       = max(dot(n, lightDir), 0.0);
    vec3  reflectDir = reflect(-lightDir, n);
    float spec       = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance   = length(light.position - fragPos);
    float att        = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 ambient  = light.ambientColor  * material.ambientColor  * att;
    vec3 diffuse  = light.diffuseColor  * diff * material.diffuseColor  * att;
    vec3 specular = light.specularColor * spec * material.specularColor * att;

    float shadow = ivfShadowFactor(light.shadowIndex, fragPos, normalize(normal), lightDir);
    return ambient + (1.0 - shadow * light.shadowStrength) * (diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir)
{
    vec3 n = normalize(norm);
    n = dot(n, viewDir) < -0.1 ? -n : n;

    vec3  lightDir   = normalize(light.position - fragPos);
    float diff       = max(dot(n, lightDir), 0.0);
    vec3  reflectDir = reflect(-lightDir, n);
    float spec       = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance   = length(light.position - fragPos);
    float att        = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    float theta      = dot(lightDir, normalize(-light.direction));
    float epsilon    = light.cutOff - light.outerCutOff;
    float intensity  = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient  = light.ambientColor  * material.ambientColor  * att * intensity;
    vec3 diffuse  = light.diffuseColor  * diff * material.diffuseColor  * att * intensity;
    vec3 specular = light.specularColor * spec * material.specularColor * att * intensity;

    float shadow = ivfShadowFactor(light.shadowIndex, fragPos, normalize(normal), lightDir);
    return ambient + (1.0 - shadow * light.shadowStrength) * (diffuse + specular);
}

vec4 applyTexBlendMode(vec4 textureColor, vec4 baseColor)
{
    vec4 result;
    switch (blendMode) {
        case TEX_BLEND_MULTIPLY:
            result = textureColor * baseColor; break;
        case TEX_BLEND_ADD:
            result = min(textureColor + baseColor, vec4(1.0)); break;
        case TEX_BLEND_SCREEN:
            result = vec4(1.0) - (vec4(1.0) - textureColor) * (vec4(1.0) - baseColor); break;
        case TEX_BLEND_OVERLAY:
            result = vec4(
                baseColor.r < 0.5 ? 2.0*baseColor.r*textureColor.r : 1.0 - 2.0*(1.0-baseColor.r)*(1.0-textureColor.r),
                baseColor.g < 0.5 ? 2.0*baseColor.g*textureColor.g : 1.0 - 2.0*(1.0-baseColor.g)*(1.0-textureColor.g),
                baseColor.b < 0.5 ? 2.0*baseColor.b*textureColor.b : 1.0 - 2.0*(1.0-baseColor.b)*(1.0-textureColor.b),
                baseColor.a); break;
        case TEX_BLEND_DECAL:
            result = vec4(mix(baseColor.rgb, textureColor.rgb, textureColor.a), baseColor.a); break;
        case TEX_BLEND_NORMAL:
        default:
            result = textureColor; break;
    }
    return mix(baseColor, result, blendFactor);
}

vec4 applyTexBlendModeIndexed(vec4 textureColor, vec4 baseColor, int mode, float factor)
{
    vec4 result;
    switch (mode) {
        case TEX_BLEND_MULTIPLY:
            result = textureColor * baseColor; break;
        case TEX_BLEND_ADD:
            result = min(textureColor + baseColor, vec4(1.0)); break;
        case TEX_BLEND_SCREEN:
            result = vec4(1.0) - (vec4(1.0) - textureColor) * (vec4(1.0) - baseColor); break;
        case TEX_BLEND_OVERLAY:
            result = vec4(
                baseColor.r < 0.5 ? 2.0*baseColor.r*textureColor.r : 1.0 - 2.0*(1.0-baseColor.r)*(1.0-textureColor.r),
                baseColor.g < 0.5 ? 2.0*baseColor.g*textureColor.g : 1.0 - 2.0*(1.0-baseColor.g)*(1.0-textureColor.g),
                baseColor.b < 0.5 ? 2.0*baseColor.b*textureColor.b : 1.0 - 2.0*(1.0-baseColor.b)*(1.0-textureColor.b),
                baseColor.a); break;
        case TEX_BLEND_DECAL:
            result = vec4(mix(baseColor.rgb, textureColor.rgb, textureColor.a), baseColor.a); break;
        case TEX_BLEND_NORMAL:
        default:
            result = textureColor; break;
    }
    return mix(baseColor, result, factor);
}

)";

} // namespace ivf
