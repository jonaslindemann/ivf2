#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;
uniform float pulse;

out vec3 worldPos;
out vec3 worldNormal;

void main()
{
    // Breathe along the normal
    float s = 1.0 + pulse * 0.08 * sin(time * 2.0 + aPos.y * 4.0);
    vec4 p = model * vec4(aPos * s, 1.0);
    worldPos = p.xyz;
    worldNormal = normalize(mat3(model) * aNormal);
    gl_Position = projection * view * p;
}
