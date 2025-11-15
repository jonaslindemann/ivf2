#pragma once

/**
 * @file proc_utils.h
 * @brief Procedural texture generation utilities for the ivf library.
 */

#include <vector>
#include <cmath>
#include <glm/glm.hpp>

namespace ivf {

/**
 * @namespace ProcUtils
 * @brief Utility functions for procedural texture generation.
 *
 * This namespace provides various noise functions and pattern generators
 * that can be used to create procedural textures.
 */
namespace ProcUtils {

/**
 * @brief Simple pseudo-random hash function for procedural generation.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return float Pseudo-random value in [0, 1].
 */
float hash(int x, int y);

/**
 * @brief 2D Perlin-style noise function.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return float Noise value in approximately [-1, 1].
 */
float noise(float x, float y);

/**
 * @brief Fractional Brownian Motion (fBm) - layered noise.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param octaves Number of noise layers to combine.
 * @param persistence Amplitude multiplier for each octave (typically 0.5).
 * @param lacunarity Frequency multiplier for each octave (typically 2.0).
 * @return float Combined noise value.
 */
float fbm(float x, float y, int octaves = 4, float persistence = 0.5f, float lacunarity = 2.0f);

/**
 * @brief Smooth interpolation function (smoothstep).
 * @param t Input value.
 * @return float Smoothed value.
 */
float smoothstep(float t);

/**
 * @brief Bilinear interpolation.
 * @param a Value at (0,0).
 * @param b Value at (1,0).
 * @param c Value at (0,1).
 * @param d Value at (1,1).
 * @param tx Interpolation factor in x.
 * @param ty Interpolation factor in y.
 * @return float Interpolated value.
 */
float bilerp(float a, float b, float c, float d, float tx, float ty);

/**
 * @brief Simple 2D Voronoi/Worley noise.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param cellSize Size of Voronoi cells.
 * @return float Distance to nearest cell center.
 */
float voronoi(float x, float y, float cellSize = 1.0f);

/**
 * @brief Turbulence function (absolute value of noise).
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param octaves Number of octaves.
 * @return float Turbulence value.
 */
float turbulence(float x, float y, int octaves = 4);

/**
 * @brief Marble pattern using sine and noise.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param scale Pattern scale.
 * @param octaves Number of noise octaves.
 * @return float Marble pattern value in [0, 1].
 */
float marble(float x, float y, float scale = 1.0f, int octaves = 4);

/**
 * @brief Wood grain pattern using sine and noise.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param scale Pattern scale.
 * @param rings Number of rings.
 * @return float Wood pattern value in [0, 1].
 */
float wood(float x, float y, float scale = 1.0f, float rings = 10.0f);

/**
 * @brief Checkerboard pattern.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param size Size of each checker.
 * @return float 0.0 or 1.0 for black/white.
 */
float checkerboard(float x, float y, float size = 1.0f);

/**
 * @brief Gradient pattern.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param angle Gradient angle in radians.
 * @return float Gradient value in [0, 1].
 */
float gradient(float x, float y, float angle = 0.0f);

/**
 * @brief Radial gradient pattern.
 * @param x X coordinate (normalized to [0, 1]).
 * @param y Y coordinate (normalized to [0, 1]).
 * @param centerX Center X position.
 * @param centerY Center Y position.
 * @return float Distance from center, clamped to [0, 1].
 */
float radialGradient(float x, float y, float centerX = 0.5f, float centerY = 0.5f);

/**
 * @brief Mix two colors using a factor.
 * @param color1 First color.
 * @param color2 Second color.
 * @param factor Mix factor in [0, 1].
 * @return glm::vec4 Mixed color.
 */
glm::vec4 mixColors(const glm::vec4 &color1, const glm::vec4 &color2, float factor);

/**
 * @brief Clamp a value to [0, 1] range.
 * @param value Input value.
 * @return float Clamped value.
 */
float clamp01(float value);

/**
 * @brief Map a value from one range to another.
 * @param value Input value.
 * @param inMin Input minimum.
 * @param inMax Input maximum.
 * @param outMin Output minimum.
 * @param outMax Output maximum.
 * @return float Mapped value.
 */
float remap(float value, float inMin, float inMax, float outMin, float outMax);

} // namespace ProcUtils

} // namespace ivf
