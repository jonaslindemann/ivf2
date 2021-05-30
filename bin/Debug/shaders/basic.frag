#version 330 core
out vec4 fragColor;

in vec3 normal;  
in vec3 fragPos;  
in vec4 color;
in vec2 texCoord;
  
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec4 lightColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform vec4 ambientColor;
uniform bool useLighting;
uniform bool useTexture;
uniform bool useVertexColors;
uniform bool usePointFalloff = false;
uniform float point_falloff_a = 0.0;
uniform float point_falloff_b = 0.0;

uniform sampler2D texture0;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{	
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor.rgb;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
    vec3 specular = specularStrength * spec * lightColor.rgb;  

    // Distance reduction

    float intensity = 1.0;

    if (usePointFalloff)
    {
	    float dist = length(lightDir);
	    float a = point_falloff_a;
	    float b = point_falloff_b;
	    intensity = 1.0f / (a *  dist * dist + b * dist + 1.0f);
    }

    vec3 result;

    if (useVertexColors)
        result = (ambientColor.rgb + diffuse + specular) * color.rgb * intensity;
    else
        result = (ambientColor.rgb + diffuse*diffuseColor.rgb + specular*specularColor.rgb) * intensity;

    if (useLighting)
        if (useTexture)
            fragColor = texture(texture0, texCoord)*vec4(result, 1.0);
        else
            fragColor = vec4(result, 1.0);
    else
        if (useVertexColors)
            fragColor = color;
        else
            fragColor = diffuseColor;
} 