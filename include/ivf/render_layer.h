#pragma once
/**
 * @file render_layer.h
 * @brief Draw order groups of the render queue.
 */

#include <cstdint>

namespace ivf {

/**
 * @brief Group a draw item belongs to; groups are drawn in this order.
 *
 * Within Opaque, items are sorted by shader program and material, then front to back;
 * within Transparent, back to front (drawn with alpha blending and without depth writes).
 */
enum class RenderLayer : std::uint8_t {
    Background,  ///< Drawn first (skyboxes, sky domes).
    Opaque,      ///< Default for solid geometry.
    Transparent, ///< Materials with alpha < 1, blended materials, particles, billboards.
    Overlay      ///< Drawn last (helpers, gizmos).
};

} // namespace ivf
