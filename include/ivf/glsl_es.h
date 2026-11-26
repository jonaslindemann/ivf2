#pragma once

/**
 * @file glsl_es.h
 * @brief Translation of desktop GLSL (3.30-4.10 core) to GLSL ES 3.00 for OpenGL ES 3 / WebGL 2.
 *
 * ivf2's shaders are written once, as desktop GLSL. When the render context runs on OpenGL ES
 * (RenderContext::isGles(), e.g. in a browser), prepareShaderSource() rewrites them before
 * compiling:
 *
 * - the `#version` line becomes `#version 300 es`, followed by default precisions (high
 *   precision for floats, ints and every sampler type) and a `#line` directive, so compiler
 *   messages keep the original line numbers;
 * - uniform initialisers (`uniform float gamma = 1.0;`), which GLSL ES does not allow, are
 *   removed and returned as UniformDefault values; Program::link() sets them after linking
 *   (applyUniformDefaults()), so the uniforms start with the same values as on desktop GL.
 *
 * Everything else must already be valid in both dialects: no `layout(binding = n)`, no double
 * precision, no geometry shaders, integer and float values not mixed without conversion.
 */

#include <string>
#include <string_view>
#include <vector>

namespace ivf {

/** @brief Initial value of a uniform declared with an initialiser, e.g. `uniform vec3 up = vec3(0, 1, 0);`. */
struct UniformDefault {
    std::string type;          ///< GLSL type: float, int, uint, bool or a vector of them (vec3, ivec2, ...).
    std::string name;          ///< Uniform name.
    int arraySize = 0;         ///< Number of elements of an array uniform; 0 for a single value.
    std::vector<double> values; ///< All components of all elements, in order.
};

/** @brief Result of translateToGlslEs(). */
struct GlslEsTranslation {
    std::string source;                  ///< GLSL ES 3.00 source.
    std::vector<UniformDefault> defaults; ///< Removed uniform initialisers.
    std::vector<std::string> warnings;    ///< Initialisers that could not be evaluated (removed anyway).
};

/**
 * @brief Translate a desktop GLSL shader to GLSL ES 3.00 (no OpenGL needed).
 * @param source Shader source starting with a `#version` line (one is added if missing).
 */
[[nodiscard]] GlslEsTranslation translateToGlslEs(std::string_view source);

/**
 * @brief The source to compile in the current GL: unchanged on desktop OpenGL, translated by
 *        translateToGlslEs() on OpenGL ES (RenderContext::isGles()).
 * @param source Desktop GLSL source.
 * @param defaults If not null, receives the uniform initialisers removed by the translation
 *        (empty on desktop GL). Pass them to applyUniformDefaults() after linking.
 */
[[nodiscard]] std::string prepareShaderSource(std::string_view source, std::vector<UniformDefault> *defaults = nullptr);

/**
 * @brief Set uniforms of a linked program to their declared initial values.
 * @param program OpenGL program object; uniforms it does not use are skipped.
 */
void applyUniformDefaults(unsigned int program, const std::vector<UniformDefault> &defaults);

} // namespace ivf
