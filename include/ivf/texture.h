#pragma once

#include <string>

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
    bool load(const std::string filename);

    /**
     * @brief Unbind the texture from the current OpenGL context.
     */
    void unbind();

    /**
     * @brief Set the internal format of the texture.
     * @param format Internal format (e.g., GL_RGBA).
     */
    void setIntFormat(GLint format);

    /**
     * @brief Set the format of the pixel data.
     * @param format Format (e.g., GL_RGBA).
     */
    void setFormat(GLenum format);

    /**
     * @brief Set the data type of the pixel data.
     * @param type Data type (e.g., GL_UNSIGNED_BYTE).
     */
    void setType(GLenum type);

    /**
     * @brief Set the mipmap level.
     * @param level Mipmap level.
     */
    void setLevel(GLint level);

    /**
     * @brief Set the texture unit.
     * @param unit Texture unit index.
     */
    void setTexUnit(GLint unit);

    /**
     * @brief Enable or disable the use of a local blend mode.
     * @param flag True to use local blend mode, false otherwise.
     */
    void setUseLocalBlendMode(bool flag);

    /**
     * @brief Set the blend mode for this texture.
     * @param blendMode Blend mode value.
     */
    void setBlendMode(TextureBlendMode blendMode);

    /**
     * @brief Set the blend factor for this texture.
     * @param blendFactor Blend factor value.
     */
    void setBlendFactor(float blendFactor);

    /**
     * @brief Set the texture wrapping mode for the T coordinate.
     * @param wrapT Wrapping mode (e.g., GL_REPEAT).
     */
    void setWrapT(GLint wrapT);

    /**
     * @brief Set the texture wrapping mode for the S coordinate.
     * @param wrapS Wrapping mode (e.g., GL_REPEAT).
     */
    void setWrapS(GLint wrapS);

    /**
     * @brief Set the minification filter.
     * @param minFilter Minification filter (e.g., GL_LINEAR).
     */
    void setMinFilter(GLint minFilter);

    /**
     * @brief Set the magnification filter.
     * @param magFilter Magnification filter (e.g., GL_LINEAR).
     */
    void setMagFilter(GLint magFilter);

    /**
     * @brief Get whether a local blend mode is used.
     * @return GLint 1 if local blend mode is used, 0 otherwise.
     */
    GLint useLocalBlendMode();

    /**
     * @brief Get the blend mode for this texture.
     * @return TextureBlendMode Blend mode value.
     */
    TextureBlendMode blendMode();

    /**
     * @brief Get the blend factor for this texture.
     * @return float Blend factor value.
     */
    float blendFactor();

    /**
     * @brief Get the texture unit index.
     * @return GLint Texture unit index.
     */
    GLint texUnit();

    /**
     * @brief Get the OpenGL texture object ID.
     * @return GLuint Texture object ID.
     */
    GLuint id();
};

/**
 * @typedef TexturePtr
 * @brief Shared pointer type for Texture.
 */
typedef std::shared_ptr<Texture> TexturePtr;

}; // namespace ivf
