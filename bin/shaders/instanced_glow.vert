#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;
layout (location = 4) in mat4 aInstanceModel;
layout (location = 8) in vec4 aInstanceColor;
layout (location = 9) in vec4 aInstanceData;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 worldPos;
out vec3 worldNormal;
out vec4 instColor;
out float pulse;

void main()
{
    // aInstanceData.x holds a per-instance phase
    pulse = 0.5 + 0.5 * sin(time * 3.0 + aInstanceData.x);
    mat4 m = model * aInstanceModel;
    vec4 p = m * vec4(aPos * (0.8 + 0.4 * pulse), 1.0);
    worldPos = p.xyz;
    worldNormal = normalize(mat3(m) * aNormal);
    instColor = aInstanceColor;
    gl_Position = projection * view * p;
}
