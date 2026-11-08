#version 330 core
in vec3 worldPos;
in float height;

uniform vec3 glowColor;

out vec4 fragColor;

void main()
{
    float crest = smoothstep(0.2, 1.0, height);
    vec3 base = vec3(0.02, 0.03, 0.06);
    fragColor = vec4(base + glowColor * crest, 1.0);
}
