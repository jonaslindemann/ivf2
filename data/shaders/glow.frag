#version 330 core
in vec3 worldPos;
in vec3 worldNormal;

uniform vec3 viewPos;
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
