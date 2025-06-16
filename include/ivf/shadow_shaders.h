// src/ivf/shadow_shaders.h (new file)
#pragma once

#include <string>

namespace ivf {

/**
 * @file shadow_shaders.h
 * @brief Contains GLSL shader source code strings for shadow mapping depth passes.
 *
 * This header provides inline string constants for vertex and fragment shaders used in
 * shadow mapping. The vertex shader transforms geometry into light space for depth rendering,
 * and the fragment shader is empty since only depth information is needed.
 */

/// Vertex shader for shadow map depth rendering (OpenGL 3.3+).
inline const std::string shadow_depth_vert_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";

/// Fragment shader for shadow map depth rendering (OpenGL 3.3+).
inline const std::string shadow_depth_frag_shader = R"(
#version 330 core

void main()
{
    // Depth is automatically written to the depth buffer
    // No need for color output since we only care about depth
}
)";

} // namespace ivf
