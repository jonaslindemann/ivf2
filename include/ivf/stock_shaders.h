#pragma once

#include <string>

namespace ivf {

std::string basic_vert_shader_source = R"(
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

std::string basic_frag_shader_source = R"(
#version 330 core

//
// A lot of information from:
//
// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/2.lighting/6.multiple_lights/6.multiple_lights.fs
//

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
uniform bool useTexture;
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

    if (useLighting)
        if (useTexture)
            //fragColor = vec4(texCoord, 0.0, 1.0);
            //fragColor = texture(texture0, texCoord) * vec4(1.0, 0.0, 0.0, 1.0); 
            if (textRendering)
                if (useFixedTextColor)
                    fragColor = vec4(textColor.r, textColor.g, textColor.b, texture(texture0, texCoord).r);
                else
                    fragColor = vec4(result.r, result.g, result.b, texture(texture0, texCoord).r);
            else
                fragColor = texture(texture0, texCoord) * vec4(result, 1.0);
        else
            fragColor = vec4(result, 1.0);
    else
        if (useVertexColors)
            fragColor = color;
        else
            fragColor = vec4(material.diffuseColor, 1.0);
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
})";

}; // namespace ivf
