#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @enum TextureBlendMode
 * @brief Supported blend modes for texture compositing.
 */
enum TextureBlendMode {
    Normal = 0,   ///< Standard texture blending.
    Multiply = 1, ///< Multiply blend mode.
    Add = 2,      ///< Additive blend mode.
    Screen = 3,   ///< Screen blend mode.
    Overlay = 4,  ///< Overlay blend mode.
    Decal = 5     ///< Decal blend mode (alpha compositing).
};

/**
 * @class TextureManager
 * @brief Singleton class for managing texture blend modes and usage state.
 *
 * The TextureManager class provides a global interface for managing texture blend mode,
 * blend factor, and texture usage state. It supports saving and restoring state, and
 * applies texture parameters to the rendering context or shaders as needed.
 */
class TextureManager {
    friend class RenderContext; // owns and creates the manager

private:
    TextureBlendMode m_textureBlendMode;      ///< Current texture blend mode.
    TextureBlendMode m_savedTextureBlendMode; ///< Saved blend mode for state restoration.
    GLint m_textureBlendModeId;               ///< Shader uniform location for blend mode.

    GLfloat m_blendFactor;      ///< Current blend factor.
    GLfloat m_savedBlendFactor; ///< Saved blend factor for state restoration.
    GLint m_blendFactorId;      ///< Shader uniform location for blend factor.

    GLboolean m_useTexture;      ///< Whether texture usage is enabled.
    GLboolean m_savedUseTexture; ///< Saved texture usage state.
    GLint m_useTextureId;        ///< Shader uniform location for texture usage.

    // Multitexturing support
    bool m_globalMultitexturingEnabled{true}; ///< Global enable/disable for multitexturing.

    TextureManager();                  ///< Private constructor for singleton pattern.

public:
    /**
     * @brief The TextureManager of the current RenderContext (RenderContext::current()->textures()).
     * @return TextureManager* Pointer to the manager.
     */
    static TextureManager *instance();

    /**
     * @brief Same as instance(); kept for compatibility.
     * @return TextureManager* Pointer to the manager.
     */
    static TextureManager *create();

    /**
     * @brief Deprecated no-op: managers are owned and released by their RenderContext.
     */
    static void drop();

    /**
     * @brief Set the current texture blend mode.
     * @param blendMode Blend mode value.
     */
    void setTextureBlendMode(TextureBlendMode blendMode);

    /**
     * @brief Set the blend factor for texture blending.
     * @param blendFactor Blend factor value.
     */
    void setBlendFactor(float blendFactor);

    /**
     * @brief Enable or disable texture usage.
     * @param useTexture True to enable, false to disable.
     */
    void setUseTexture(bool useTexture);

    /**
     * @brief Get the current texture blend mode.
     * @return TextureBlendMode Blend mode value.
     */
    TextureBlendMode textureBlendMode() const;

    /**
     * @brief Get the current blend factor.
     * @return GLfloat Blend factor value.
     */
    GLfloat blendFactor() const;

    /**
     * @brief Get whether texture usage is enabled.
     * @return GLboolean 1 if enabled, 0 otherwise.
     */
    GLboolean useTexture() const;

    /**
     * @brief Enable or disable global multitexturing support.
     * @param flag True to enable, false to disable.
     */
    void setGlobalMultitexturingEnabled(bool flag);

    /**
     * @brief Check if global multitexturing is enabled.
     * @return bool True if enabled.
     */
    bool globalMultitexturingEnabled() const;

    /**
     * @brief Apply the texture parameters to the rendering context or shader.
     */
    void apply();

    /**
     * @brief Save the current texture state.
     */
    void saveState();

    /**
     * @brief Restore the previously saved texture state.
     */
    void restoreState();
};

/**
 * @typedef TextureManagerPtr
 * @brief Pointer type for TextureManager singleton.
 */
typedef TextureManager *TextureManagerPtr;

}; // namespace ivf
