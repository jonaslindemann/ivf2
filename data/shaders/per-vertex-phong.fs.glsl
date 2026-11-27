#version 410 core

// Output
layout (location = 0) out vec4 color;

// Input from vertex shader
in vec3 vs_color;

void main(void)
{
    // Write incoming color to the framebuffer
    color = vec4(vs_color, 1.0);
}
