#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;
uniform float amplitude;

out vec3 worldPos;
out float height;

void main()
{
    vec4 p = model * vec4(aPos, 1.0);
    float r = length(p.xz);
    height = sin(r * 2.0 - time * 3.0) * exp(-r * 0.12);
    p.y += amplitude * height;
    worldPos = p.xyz;
    gl_Position = projection * view * p;
}
