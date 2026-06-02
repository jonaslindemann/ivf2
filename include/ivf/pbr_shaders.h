#pragma once

#include <string>

namespace ivf {

/// Vertex shader for PBR rendering (OpenGL 3.3+).
/// Uses identical attribute layout to the basic vertex shader so existing Mesh VAOs work unchanged.
inline const std::string pbr_vert_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

out vec3 fragPos;
out vec3 vNormal;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragPos  = vec3(model * vec4(aPos, 1.0));
    vNormal  = mat3(model) * aNormal;
    texCoord = aTex;
    gl_Position = projection * view * vec4(fragPos, 1.0);
}
)";

/// Fragment shader for PBR rendering with Cook-Torrance BRDF (OpenGL 3.3+).
/// Supports point, directional and spot lights (same uniform names as basic shader),
/// optional PBR texture maps, optional environment cubemap reflection, and
/// derivative-based TBN for normal mapping without extra vertex attributes.
inline const std::string pbr_frag_shader_source = R"(
#version 330 core

out vec4 fragColor;

// ---- Inputs ----------------------------------------------------------------
in vec3 fragPos;
in vec3 vNormal;
in vec2 texCoord;

// ---- Light structs (identical names to basic shader so LightManager works) -

struct DirLight
{
    bool  enabled;
    vec3  direction;
    vec3  ambientColor;
    vec3  diffuseColor;
    vec3  specularColor;
    bool  castShadows;
    mat4  lightSpaceMatrix;
};

struct PointLight
{
    bool  enabled;
    vec3  position;
    float constant;
    float linear;
    float quadratic;
    vec3  ambientColor;
    vec3  diffuseColor;
    vec3  specularColor;
};

struct SpotLight
{
    bool  enabled;
    vec3  position;
    vec3  direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3  ambientColor;
    vec3  diffuseColor;
    vec3  specularColor;
};

// ---- Lighting uniforms (shared with basic shader) --------------------------
#define NR_POINT_LIGHTS 8
#define NR_DIR_LIGHTS   8
#define NR_SPOT_LIGHTS  8

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight   dirLights[NR_DIR_LIGHTS];
uniform SpotLight  spotLights[NR_SPOT_LIGHTS];

uniform int  pointLightCount = 0;
uniform int  dirLightCount   = 0;
uniform int  spotLightCount  = 0;

uniform bool useLighting     = true;
uniform bool useVertexColors = false;

uniform vec3 viewPos;

// ---- Selection (identical to basic shader) ---------------------------------
uniform bool selectionRendering = false;
uniform uint objectId;

// ---- PBR material ----------------------------------------------------------
struct PBRMaterial
{
    vec4  albedo;
    float roughness;
    float metallic;
    vec3  emissive;
    float ao;
};
uniform PBRMaterial pbr;

// ---- PBR texture maps ------------------------------------------------------
uniform bool      useAlbedoMap    = false;
uniform bool      useNormalMap    = false;
uniform bool      useRoughnessMap = false;
uniform bool      useMetallicMap  = false;
uniform bool      useAOMap        = false;
uniform bool      useEnvCubemap   = false;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform sampler2D aoMap;
uniform samplerCube envCubemap;

// ---- Constants -------------------------------------------------------------
const float PI = 3.14159265359;

// ---- Cook-Torrance BRDF helpers --------------------------------------------

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0001);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ---- Derivative-based TBN reconstruction -----------------------------------
// No tangent vertex attribute needed; derived purely from screen-space derivatives.
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

// ---- Per-light radiance evaluation -----------------------------------------
vec3 evalBRDF(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo, float roughness, float metallic, vec3 F0)
{
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 numerator   = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    vec3 specular = numerator / denominator;

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// ---- Main ------------------------------------------------------------------
void main()
{
    // Selection pass
    if (selectionRendering)
    {
        uint mask = uint(255);
        uint r = objectId & mask;
        uint g = (objectId >> uint(8)) & mask;
        uint b = (objectId >> uint(16)) & mask;
        fragColor = vec4(float(r)/255.0, float(g)/255.0, float(b)/255.0, 1.0);
        return;
    }

    // --- Resolve material values from maps or scalars ---
    vec4  albedo4   = useAlbedoMap    ? texture(albedoMap, texCoord)    : pbr.albedo;
    vec3  albedo    = pow(albedo4.rgb, vec3(2.2)); // gamma to linear
    float roughness = useRoughnessMap ? texture(roughnessMap, texCoord).r : pbr.roughness;
    float metallic  = useMetallicMap  ? texture(metallicMap, texCoord).r  : pbr.metallic;
    float ao        = useAOMap        ? texture(aoMap, texCoord).r         : pbr.ao;

    roughness = clamp(roughness, 0.04, 1.0);
    metallic  = clamp(metallic,  0.0,  1.0);

    // --- Normal ---
    vec3 N = normalize(vNormal);
    if (useNormalMap)
    {
        mat3 TBN    = buildTBN(N);
        vec3 tnorm  = texture(normalMap, texCoord).rgb * 2.0 - 1.0;
        N = normalize(TBN * tnorm);
    }

    vec3 V  = normalize(viewPos - fragPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    if (useLighting)
    {
        // --- Directional lights ---
        for (int i = 0; i < dirLightCount; ++i)
        {
            if (!dirLights[i].enabled) continue;
            vec3 L        = normalize(-dirLights[i].direction);
            vec3 radiance = dirLights[i].diffuseColor;
            Lo += evalBRDF(N, V, L, radiance, albedo, roughness, metallic, F0);
        }

        // --- Point lights ---
        for (int i = 0; i < pointLightCount; ++i)
        {
            if (!pointLights[i].enabled) continue;
            vec3  L        = normalize(pointLights[i].position - fragPos);
            float dist     = length(pointLights[i].position - fragPos);
            float att      = 1.0 / (pointLights[i].constant
                                  + pointLights[i].linear * dist
                                  + pointLights[i].quadratic * dist * dist);
            vec3 radiance  = pointLights[i].diffuseColor * att;
            Lo += evalBRDF(N, V, L, radiance, albedo, roughness, metallic, F0);
        }

        // --- Spot lights ---
        for (int i = 0; i < spotLightCount; ++i)
        {
            if (!spotLights[i].enabled) continue;
            vec3  L        = normalize(spotLights[i].position - fragPos);
            float dist     = length(spotLights[i].position - fragPos);
            float att      = 1.0 / (spotLights[i].constant
                                  + spotLights[i].linear * dist
                                  + spotLights[i].quadratic * dist * dist);
            float theta    = dot(L, normalize(-spotLights[i].direction));
            float epsilon  = spotLights[i].cutOff - spotLights[i].outerCutOff;
            float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);
            vec3 radiance  = spotLights[i].diffuseColor * att * intensity;
            Lo += evalBRDF(N, V, L, radiance, albedo, roughness, metallic, F0);
        }
    }

    // --- Ambient / environment ---
    vec3 ambient;
    if (useEnvCubemap)
    {
        vec3 R = reflect(-V, N);
        float lod = roughness * 4.0; // crude mip selection
        vec3 envColor = texture(envCubemap, R).rgb;
        vec3 F = fresnelSchlick(max(dot(N, V), 0.0), F0);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        ambient = (kD * albedo + F * envColor) * ao;
    }
    else
    {
        ambient = vec3(0.03) * albedo * ao;
    }

    vec3 outColor = ambient + Lo + pbr.emissive;

    // Tone mapping (Reinhard) + gamma correction
    outColor = outColor / (outColor + vec3(1.0));
    outColor = pow(outColor, vec3(1.0 / 2.2));

    fragColor = vec4(outColor, albedo4.a);
}
)";

} // namespace ivf
