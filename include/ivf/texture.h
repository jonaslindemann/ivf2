#pragma once

#include <string>
#include <string_view>

#include <ivf/glbase.h>
#include <ivf/field.h>
#include <stb_image.h>

#include <ivf/texture_manager.h>

namespace ivf {

/**
 * @class Texture
 * @brief Manages an OpenGL texture object, including loading, binding, and parameters.
 *
 * The Texture class encapsulates the creation, loading, and management of OpenGL texture objects.
 * It supports loading image data from files, setting texture parameters (format, filtering, wrapping),
 * and configuring blend modes for advanced rendering. The class provides methods to bind/unbind
 * the texture and query or set its properties.
 */
class Texture : public GLBase {
private:
    GLuint m_id;       ///< OpenGL texture object ID.
    GLint m_wrapT;     ///< Texture wrapping mode for T coordinate.
    GLint m_wrapS;     ///< Texture wrapping mode for S coordinate.
    GLint m_minFilter; ///< Minification filter.
    GLint m_magFilter; ///< Magnification filter.
    GLint m_intFormat; ///< Internal format.
    GLint m_level;     ///< Mipmap level.
    GLint m_texUnit;   ///< Texture unit.
    GLenum m_format;   ///< Format of the pixel data.
    GLenum m_type;     ///< Data type of the pixel data.

    bool m_useLocalBlendMode;     ///< Whether to use a local blend mode.
    TextureBlendMode m_blendMode; ///< Blend mode for this texture.
    float m_blendFactor;          ///< Blend factor for blending.

public:
    /**
     * @brief Constructor.
     */
    Texture();

    /**
     * @brief Destructor.
     */
    virtual ~Texture();

    /**
     * @brief Factory method to create a shared pointer to a Texture instance.
     * @return std::shared_ptr<Texture> New Texture instance.
     */
    static std::shared_ptr<Texture> create();

    /**
     * @brief Bind the texture to the current OpenGL context.
     */
    void bind();

    /**
     * @brief Load texture data from an image file.
     * @param filename Path to the image file.
     * @return bool True if loading succeeded.
     */
    bool load(std::string_view filename);

    /**
     * @brief Unbind the texture from the current OpenGL context.
     */
    void unbind();

    /**
     * @brief Set the internal format of the texture.
     * @param format Internal format (e.g., GL_RGBA).
     */
    inline void setIntFormat(GLint format) noexcept { m_intFormat = format; }

    /**
     * @brief Set the format of the pixel data.
     * @param format Format (e.g., GL_RGBA).
     */
    inline void setFormat(GLenum format) noexcept { m_format = format; }

    /**
     * @brief Set the data type of the pixel data.
     * @param type Data type (e.g., GL_UNSIGNED_BYTE).
     */
    inline void setType(GLenum type) noexcept { m_type = type; }

    /**
     * @brief Set the mipmap level.
     * @param level Mipmap level.
     */
    inline void setLevel(GLint level) noexcept { m_level = level; }

    /**
     * @brief Set the texture unit.
     * @param unit Texture unit index.
     */
    inline void setTexUnit(GLint unit) noexcept { m_texUnit = unit; }

    /**
     * @brief Enable or disable the use of a local blend mode.
     * @param flag True to use local blend mode, false otherwise.
     */
    inline void setUseLocalBlendMode(bool flag) noexcept { m_useLocalBlendMode = flag; }

    /**
     * @brief Set the blend mode for this texture.
     * @param blendMode Blend mode value.
     */
    inline void setBlendMode(TextureBlendMode blendMode) noexcept { m_blendMode = blendMode; }

    /**
     * @brief Set the blend factor for this texture.
     * @param blendFactor Blend factor value.
     */
    inline void setBlendFactor(float blendFactor) noexcept { m_blendFactor = blendFactor; }

    /**
     * @brief Set the texture wrapping mode for the T coordinate.
     * @param wrapT Wrapping mode (e.g., GL_REPEAT).
     */
    inline void setWrapT(GLint wrapT) noexcept { m_wrapT = wrapT; }

    /**
     * @brief Set the texture wrapping mode for the S coordinate.
     * @param wrapS Wrapping mode (e.g., GL_REPEAT).
     */
    inline void setWrapS(GLint wrapS) noexcept { m_wrapS = wrapS; }

    /**
     * @brief Set the minification filter.
     * @param minFilter Minification filter (e.g., GL_LINEAR).
     */
    inline void setMinFilter(GLint minFilter) noexcept { m_minFilter = minFilter; }

    /**
     * @brief Set the magnification filter.
     * @param magFilter Magnification filter (e.g., GL_LINEAR).
     */
    inline void setMagFilter(GLint magFilter) noexcept { m_magFilter = magFilter; }

    /**
     * @brief Get whether a local blend mode is used.
     * @return GLint 1 if local blend mode is used, 0 otherwise.
     */
    [[nodiscard]] inline GLint useLocalBlendMode() const noexcept { return m_useLocalBlendMode ? 1 : 0; }

    /**
     * @brief Get the blend mode for this texture.
     * @return TextureBlendMode Blend mode value.
     */
    [[nodiscard]] inline TextureBlendMode blendMode() const noexcept { return m_blendMode; }

    /**
     * @brief Get the blend factor for this texture.
     * @return float Blend factor value.
     */
    [[nodiscard]] inline float blendFactor() const noexcept { return m_blendFactor; }

    /**
     * @brief Get the texture unit index.
     * @return GLint Texture unit index.
     */
    [[nodiscard]] inline GLint texUnit() const noexcept { return m_texUnit; }

    /**
     * @brief Get the OpenGL texture object ID.
     * @return GLuint Texture object ID.
     */
    [[nodiscard]] inline GLuint id() const noexcept { return m_id; }
};

/**
 * @typedef TexturePtr
 * @brief Shared pointer type for Texture.
 */
using TexturePtr = std::shared_ptr<Texture>;

}; // namespace ivf
