#pragma once

/**
 * @file blend_shader_material.h
 * @brief Shader material with additive or alpha blending, for glowing particles and veils.
 */

#include <ivf/shader_material.h>

#include <memory>
#include <string>

namespace ivf {

/**
 * @brief How a BlendShaderMaterial combines its output with the framebuffer.
 */
enum class BlendMode {
    Additive,  ///< Overlapping light adds up (glowing particles, light trails).
    Alpha,     ///< Translucent surfaces (veils, mist).
    Background ///< Opaque without depth writes, for a sky dome drawn first.
};

/**
 * @class BlendShaderMaterial
 * @brief ShaderMaterial that draws with blending and without depth writes.
 *
 * Additive blending makes overlapping light add up; alpha blending gives
 * translucent surfaces. Depth testing stays on, so blended geometry is hidden
 * behind opaque geometry. Nodes using it are in the Transparent render layer (Background
 * for background mode), so the render queue draws them after the opaque nodes, back to
 * front, whatever order they were added in.
 *
 * Background mode draws opaque without depth writes, for a sky dome added first:
 * everything drawn later covers it, and the depth buffer still reads as empty sky
 * for depth-based post effects such as FogEffect.
 *
 * The previous blend and depth-write state is restored after drawing.
 *
 * @code
 * auto glow = BlendShaderMaterial::createFromFiles("shaders/mote.vert", "shaders/mote.frag",
 *                                                  "mote", BlendMode::Additive, true);
 * particles->setMaterial(glow);
 * @endcode
 */
class BlendShaderMaterial : public ShaderMaterial {
public:
    /**
     * @brief Constructor.
     * @param program Shader program to render with.
     * @param mode    Blend mode.
     */
    explicit BlendShaderMaterial(ProgramPtr program = nullptr, BlendMode mode = BlendMode::Additive);

    /**
     * @brief Factory method.
     * @param program Shader program to render with.
     * @param mode    Blend mode.
     */
    [[nodiscard]] static std::shared_ptr<BlendShaderMaterial> create(ProgramPtr program = nullptr,
                                                       BlendMode mode = BlendMode::Additive);

    /**
     * @brief Load a program from vertex and fragment shader files.
     *
     * The program is registered with ShaderManager under @p name.
     *
     * @param vertPath Vertex shader path.
     * @param fragPath Fragment shader path.
     * @param name     Program name.
     * @param mode     Blend mode.
     * @param watch    If true, the files are reloaded on change via ShaderWatcher.
     * @return The material, or nullptr if the shaders failed to compile or link.
     */
    [[nodiscard]] static std::shared_ptr<BlendShaderMaterial> createFromFiles(const std::string &vertPath,
                                                                const std::string &fragPath, const std::string &name,
                                                                BlendMode mode = BlendMode::Additive,
                                                                bool watch = false);

    /**
     * @brief Create a program from vertex and fragment shader source strings.
     *
     * The program is registered with ShaderManager under @p name.
     * @return The material, or nullptr if the shaders failed to compile or link.
     */
    [[nodiscard]] static std::shared_ptr<BlendShaderMaterial> createFromStrings(const std::string &vertSource,
                                                                  const std::string &fragSource,
                                                                  const std::string &name,
                                                                  BlendMode mode = BlendMode::Additive);

    void setBlendMode(BlendMode mode);
    [[nodiscard]] BlendMode blendMode() const;

    void apply() override;
    void unapply() override;

    /// Additive and alpha blending: Transparent; background mode: Background (drawn first).
    [[nodiscard]] RenderLayer renderLayer() const override;

private:
    BlendMode m_mode;
    bool m_blendWasEnabled{false};
    unsigned char m_depthMaskWas{1};
    int m_srcRgb{0}, m_dstRgb{0}, m_srcAlpha{0}, m_dstAlpha{0};
};

using BlendShaderMaterialPtr = std::shared_ptr<BlendShaderMaterial>;

} // namespace ivf
