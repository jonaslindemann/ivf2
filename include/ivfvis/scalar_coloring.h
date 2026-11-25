#pragma once

/**
 * @file scalar_coloring.h
 * @brief Colour meshes by scalar values through a colormap (vertex colours).
 */

#include <ivfvis/colormap.h>
#include <ivfvis/uniform_grid.h>

#include <ivf/mesh.h>
#include <ivf/mesh_node.h>

#include <glm/glm.hpp>

#include <functional>
#include <span>
#include <string_view>

namespace ivfvis {

/**
 * @brief Set the vertex colours of @p mesh from @p values (one per vertex) through @p colormap.
 *
 * Extra values are ignored; vertices without a value keep their colour. The colours are
 * uploaded before the next draw (Mesh::updateColors()). The node's material must use vertex
 * colours: see useVertexColors().
 */
void colorVertices(ivf::Mesh &mesh, std::span<const float> values, const Colormap &colormap);

/**
 * @brief Colour every vertex of the meshes of @p node by @p value, a function of the vertex
 * position in the node's own (mesh) coordinates.
 *
 * @code
 * auto surface = ivf::Plane::create(4.0, 4.0, 64, 64);
 * ivfvis::colorVertices(*surface, [](const glm::vec3 &p) { return p.x * p.z; }, *cmap);
 * ivfvis::useVertexColors(*surface);
 * @endcode
 */
void colorVertices(ivf::MeshNode &node, const std::function<float(const glm::vec3 &)> &value,
                   const Colormap &colormap);

/**
 * @brief Colour the vertices of @p node by scalar array @p scalars of @p grid, sampled at
 * the vertex positions mapped into grid coordinates by @p meshToGrid.
 */
void colorVertices(ivf::MeshNode &node, const UniformGrid &grid, std::string_view scalars, const Colormap &colormap,
                   const glm::mat4 &meshToGrid = glm::mat4(1.0f));

/**
 * @brief Let @p node's material use vertex colours (Material::setUseVertexColor(),
 * PBRMaterial::setUseVertexColors()); creates a default material if the node has none.
 */
void useVertexColors(ivf::Node &node, bool flag = true);

} // namespace ivfvis
