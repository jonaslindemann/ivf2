#pragma once

/**
 * @file uniform_blocks.h
 * @brief Uniform blocks (std140 uniform buffers) shared by all shader programs.
 *
 * Data that is the same for every program in a pass - the camera today, the lights
 * later - lives in a uniform buffer owned by the render context instead of being
 * uploaded to each program. Program::link() connects a program that declares one of
 * these blocks to its fixed binding point, so shaders only declare the block:
 *
 * @code{.glsl}
 * #version 330 core
 * layout(std140) uniform IvfCamera {
 *     mat4 view;
 *     mat4 projection;
 *     mat4 viewProjection;
 *     vec3 viewPos;
 * };
 * uniform mat4 model;
 * ...
 *     gl_Position = projection * view * model * vec4(aPos, 1.0);
 * @endcode
 *
 * The block has no instance name, so its members are used like plain uniforms. Declare
 * it identically in every stage that uses one of its members (a program cannot have both
 * the block and a plain uniform named `view` or `viewPos`). C++ code can paste
 * IVF_CAMERA_BLOCK_GLSL into a shader string. Programs that declare plain `view`,
 * `projection` and `viewPos` uniforms instead still get them set as before.
 */

#include <glad/glad.h>

#include <glm/glm.hpp>

namespace ivf {

/// Binding point of the `IvfCamera` uniform block.
inline constexpr GLuint CameraBlockBinding = 0;

/// Name of the camera uniform block in GLSL.
inline constexpr const char *CameraBlockName = "IvfCamera";

/**
 * @brief Contents of the `IvfCamera` uniform block (std140 layout).
 *
 * Written by TransformManager whenever view, projection or view position change, and by
 * the Renderer at the start of each pass.
 */
struct CameraBlock {
    glm::mat4 view{1.0f};           ///< World to view space.
    glm::mat4 projection{1.0f};     ///< View to clip space.
    glm::mat4 viewProjection{1.0f}; ///< projection * view.
    glm::vec4 viewPos{0.0f};        ///< Camera position in world space (xyz; `vec3 viewPos` in GLSL).
};

static_assert(sizeof(CameraBlock) == 208, "CameraBlock must match the std140 layout of IvfCamera");

} // namespace ivf

/// GLSL declaration of the camera uniform block, for pasting into shader source strings.
#define IVF_CAMERA_BLOCK_GLSL                                                                                          \
    "layout(std140) uniform IvfCamera {\n"                                                                             \
    "    mat4 view;\n"                                                                                                 \
    "    mat4 projection;\n"                                                                                           \
    "    mat4 viewProjection;\n"                                                                                       \
    "    vec3 viewPos;\n"                                                                                              \
    "};\n"
