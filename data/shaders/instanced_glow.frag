#version 330 core
in vec3 worldPos;
in vec3 worldNormal;
in vec4 instColor;
in float pulse;

uniform vec3 viewPos;

out vec4 fragColor;

void main()
{
    vec3 v = normalize(viewPos - worldPos);
    float rim = pow(1.0 - max(dot(normalize(worldNormal), v), 0.0), 2.0);
    fragColor = vec4(instColor.rgb * (0.3 + pulse + 1.5 * rim), 1.0);
}
