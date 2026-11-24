#version 330 core
in vec3 worldPos;
in vec3 worldNormal;

// Camera data shared by all programs (see ivf/uniform_blocks.h)
layout(std140) uniform IvfCamera {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 viewPos;
};
uniform vec3 glowColor;
uniform float time;
uniform float pulse;

out vec4 fragColor;

void main()
{
    vec3 v = normalize(viewPos - worldPos);
    float rim = pow(1.0 - max(dot(normalize(worldNormal), v), 0.0), 2.5);
    float core = 0.25 + 0.15 * pulse * sin(time * 2.0);
    fragColor = vec4(glowColor * (core + 1.5 * rim), 1.0);
}
