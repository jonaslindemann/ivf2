#pragma once

/**
 * @file instancing.h
 * @brief Per-instance vertex attributes shared by InstancedMeshNode and automatic instancing.
 *
 * Instanced draws read one InstanceAttributes record per instance from a buffer bound to
 * fixed attribute locations. The stock and PBR shaders use them while the uniform
 * `useInstancing` is true:
 *
 * @code{.glsl}
 * layout (location = 4) in mat4 aInstanceModel; // locations 4..7
 * layout (location = 8) in vec4 aInstanceColor;
 * layout (location = 9) in vec4 aInstanceData;
 * uniform bool useInstancing = false;
 * ...
 *     mat4 modelMatrix = useInstancing ? model * aInstanceModel : model;
 * @endcode
 *
 * A shader without `useInstancing` cannot draw instances; the renderer then draws them one
 * by one (see Mesh::drawInstances()).
 */

#include <glad/glad.h>

#include <glm/glm.hpp>

namespace ivf {

class VertexArray;

/// One instance in an instance buffer.
struct InstanceAttributes {
    glm::mat4 transform{1.0f}; ///< Model matrix of the instance (`aInstanceModel`).
    glm::vec4 color{1.0f};     ///< Colour multiplied with the shaded colour (`aInstanceColor`).
    glm::vec4 data{0.0f};      ///< Free per-instance data for custom shaders (`aInstanceData`).
};

inline constexpr GLuint InstanceTransformLocation = 4; ///< mat4: locations 4..7.
inline constexpr GLuint InstanceColorLocation = 8;     ///< vec4 colour.
inline constexpr GLuint InstanceDataLocation = 9;      ///< vec4 data.

/**
 * @brief Point the instance attributes of @p vao at @p buffer (InstanceAttributes records).
 *
 * Binds @p vao and leaves no vertex array or array buffer bound afterwards.
 */
void bindInstanceAttributes(VertexArray &vao, GLuint buffer);

} // namespace ivf
