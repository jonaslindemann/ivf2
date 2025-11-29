#pragma once

/**
 * @file procedural_texture.h
 * @brief Procedural texture generation classes for the ivf library.
 */

#include <ivf/texture.h>
#include <ivf/proc_utils.h>
#include <glm/glm.hpp>
#include <vector>

namespace ivf {

/**
 * @class ProceduralTexture
 * @brief Base class for procedurally generated textures.
 *
 * ProceduralTexture extends the Texture class to support CPU-side generation
 * of texture data using mathematical functions and noise algorithms.
 */
class ProceduralTexture : public Texture {
protected:
    int m_width{512};                            ///< Texture width in pixels.
    int m_height{512};                           ///< Texture height in pixels.
    int m_channels{4};                           ///< Number of color channels (3=RGB, 4=RGBA).
    std::vector<unsigned char> m_data;           ///< Generated texture data.
    bool m_needsRegeneration{true};              ///< Flag indicating if texture needs regeneration.

    /**
     * @brief Generate the procedural texture data.
     * Must be implemented by derived classes.
     */
    virtual void generate() = 0;

    /**
     * @brief Get a pixel value at normalized coordinates.
     * @param u Horizontal coordinate in [0, 1].
     * @param v Vertical coordinate in [0, 1].
     * @return glm::vec4 RGBA color at the given coordinates.
     */
    virtual glm::vec4 getPixel(float u, float v) = 0;

    /**
     * @brief Set a pixel in the data buffer.
     * @param x Pixel x coordinate.
     * @param y Pixel y coordinate.
     * @param color RGBA color to set.
     */
    void setPixel(int x, int y, const glm::vec4 &color);

public:
    /**
     * @brief Constructor.
     */
    ProceduralTexture();

    /**
     * @brief Virtual destructor.
     */
    virtual ~ProceduralTexture();

    /**
     * @brief Set the texture resolution.
     * @param width Texture width.
     * @param height Texture height.
     */
    void setSize(int width, int height);

    /**
     * @brief Get the texture width.
     * @return int Texture width.
     */
    int width() const { return m_width; }

    /**
     * @brief Get the texture height.
     * @return int Texture height.
     */
    int height() const { return m_height; }

    /**
     * @brief Set the number of color channels.
     * @param channels Number of channels (3 for RGB, 4 for RGBA).
     */
    void setChannels(int channels);

    /**
     * @brief Regenerate and upload the texture to the GPU.
     * Call this after changing parameters to update the texture.
     */
    void regenerate();

    /**
     * @brief Upload the generated data to OpenGL.
     */
    void upload();

    /**
     * @brief Mark the texture as needing regeneration.
     */
    void setNeedsRegeneration(bool flag = true);
};

/**
 * @typedef ProceduralTexturePtr
 * @brief Shared pointer type for ProceduralTexture.
 */
using ProceduralTexturePtr = std::shared_ptr<ProceduralTexture>;

// ============================================================================
// Specific Procedural Texture Implementations
// ============================================================================

/**
 * @class CheckerboardTexture
 * @brief Checkerboard pattern texture.
 */
class CheckerboardTexture : public ProceduralTexture {
private:
    int m_checkerSize{32};                       ///< Size of each checker in pixels.
    glm::vec4 m_color1{1.0f, 1.0f, 1.0f, 1.0f}; ///< First checker color (white).
    glm::vec4 m_color2{0.0f, 0.0f, 0.0f, 1.0f}; ///< Second checker color (black).

protected:
    void generate() override;
    glm::vec4 getPixel(float u, float v) override;

public:
    /**
     * @brief Constructor.
     */
    CheckerboardTexture();

    /**
     * @brief Factory method to create a shared pointer to a CheckerboardTexture instance.
     * @return std::shared_ptr<CheckerboardTexture> New CheckerboardTexture instance.
     */
    static std::shared_ptr<CheckerboardTexture> create();

    /**
     * @brief Set the size of each checker.
     * @param size Checker size in pixels.
     */
    void setCheckerSize(int size);

    /**
     * @brief Set the two checker colors.
     * @param color1 First color.
     * @param color2 Second color.
     */
    void setColors(const glm::vec4 &color1, const glm::vec4 &color2);
};

/**
 * @typedef CheckerboardTexturePtr
 * @brief Shared pointer type for CheckerboardTexture.
 */
using CheckerboardTexturePtr = std::shared_ptr<CheckerboardTexture>;

/**
 * @class PerlinNoiseTexture
 * @brief Perlin noise texture.
 */
class PerlinNoiseTexture : public ProceduralTexture {
private:
    float m_scale{4.0f};                         ///< Noise scale.
    int m_octaves{4};                            ///< Number of noise octaves.
    float m_persistence{0.5f};                   ///< Amplitude decay per octave.
    float m_lacunarity{2.0f};                    ///< Frequency increase per octave.
    glm::vec4 m_colorLow{0.0f, 0.0f, 0.0f, 1.0f}; ///< Color for low noise values.
    glm::vec4 m_colorHigh{1.0f, 1.0f, 1.0f, 1.0f}; ///< Color for high noise values.

protected:
    void generate() override;
    glm::vec4 getPixel(float u, float v) override;

public:
    /**
     * @brief Constructor.
     */
    PerlinNoiseTexture();

    /**
     * @brief Factory method to create a shared pointer to a PerlinNoiseTexture instance.
     * @return std::shared_ptr<PerlinNoiseTexture> New PerlinNoiseTexture instance.
     */
    static std::shared_ptr<PerlinNoiseTexture> create();

    /**
     * @brief Set the noise scale.
     * @param scale Noise scale (higher = more zoomed out).
     */
    void setScale(float scale);

    /**
     * @brief Set the number of noise octaves.
     * @param octaves Number of octaves (more = more detail).
     */
    void setOctaves(int octaves);

    /**
     * @brief Set the persistence (amplitude decay).
     * @param persistence Persistence value (typically 0.5).
     */
    void setPersistence(float persistence);

    /**
     * @brief Set the lacunarity (frequency increase).
     * @param lacunarity Lacunarity value (typically 2.0).
     */
    void setLacunarity(float lacunarity);

    /**
     * @brief Set the color range for the noise.
     * @param colorLow Color for noise value -1.
     * @param colorHigh Color for noise value +1.
     */
    void setColorRange(const glm::vec4 &colorLow, const glm::vec4 &colorHigh);
};

/**
 * @typedef PerlinNoiseTexturePtr
 * @brief Shared pointer type for PerlinNoiseTexture.
 */
using PerlinNoiseTexturePtr = std::shared_ptr<PerlinNoiseTexture>;

/**
 * @class GradientTexture
 * @brief Linear or radial gradient texture.
 */
class GradientTexture : public ProceduralTexture {
public:
    enum class GradientType {
        Linear,  ///< Linear gradient.
        Radial   ///< Radial gradient from center.
    };

private:
    GradientType m_type{GradientType::Linear};
    glm::vec4 m_colorStart{0.0f, 0.0f, 0.0f, 1.0f}; ///< Start color.
    glm::vec4 m_colorEnd{1.0f, 1.0f, 1.0f, 1.0f};   ///< End color.
    float m_angle{0.0f};                             ///< Gradient angle (radians, for linear).
    glm::vec2 m_center{0.5f, 0.5f};                  ///< Center point (for radial).

protected:
    void generate() override;
    glm::vec4 getPixel(float u, float v) override;

public:
    /**
     * @brief Constructor.
     */
    GradientTexture();

    /**
     * @brief Factory method to create a shared pointer to a GradientTexture instance.
     * @return std::shared_ptr<GradientTexture> New GradientTexture instance.
     */
    static std::shared_ptr<GradientTexture> create();

    /**
     * @brief Set the gradient type.
     * @param type Gradient type (Linear or Radial).
     */
    void setType(GradientType type);

    /**
     * @brief Set the gradient colors.
     * @param colorStart Start color.
     * @param colorEnd End color.
     */
    void setColors(const glm::vec4 &colorStart, const glm::vec4 &colorEnd);

    /**
     * @brief Set the gradient angle (for linear gradients).
     * @param angle Angle in radians.
     */
    void setAngle(float angle);

    /**
     * @brief Set the center point (for radial gradients).
     * @param center Center point in normalized coordinates [0, 1].
     */
    void setCenter(const glm::vec2 &center);
};

/**
 * @typedef GradientTexturePtr
 * @brief Shared pointer type for GradientTexture.
 */
using GradientTexturePtr = std::shared_ptr<GradientTexture>;

/**
 * @class MarbleTexture
 * @brief Marble pattern texture.
 */
class MarbleTexture : public ProceduralTexture {
private:
    float m_scale{1.0f};                         ///< Pattern scale.
    int m_octaves{4};                            ///< Number of noise octaves.
    glm::vec4 m_color1{0.9f, 0.9f, 0.9f, 1.0f}; ///< Light marble color.
    glm::vec4 m_color2{0.3f, 0.3f, 0.3f, 1.0f}; ///< Dark marble color.

protected:
    void generate() override;
    glm::vec4 getPixel(float u, float v) override;

public:
    /**
     * @brief Constructor.
     */
    MarbleTexture();

    /**
     * @brief Factory method to create a shared pointer to a MarbleTexture instance.
     * @return std::shared_ptr<MarbleTexture> New MarbleTexture instance.
     */
    static std::shared_ptr<MarbleTexture> create();

    /**
     * @brief Set the pattern scale.
     * @param scale Pattern scale.
     */
    void setScale(float scale);

    /**
     * @brief Set the number of octaves.
     * @param octaves Number of octaves.
     */
    void setOctaves(int octaves);

    /**
     * @brief Set the marble colors.
     * @param color1 Light color.
     * @param color2 Dark color.
     */
    void setColors(const glm::vec4 &color1, const glm::vec4 &color2);
};

/**
 * @typedef MarbleTexturePtr
 * @brief Shared pointer type for MarbleTexture.
 */
using MarbleTexturePtr = std::shared_ptr<MarbleTexture>;

/**
 * @class WoodTexture
 * @brief Wood grain pattern texture.
 */
class WoodTexture : public ProceduralTexture {
private:
    float m_scale{1.0f};                         ///< Pattern scale.
    float m_rings{10.0f};                        ///< Number of rings.
    glm::vec4 m_color1{0.6f, 0.4f, 0.2f, 1.0f}; ///< Light wood color.
    glm::vec4 m_color2{0.3f, 0.2f, 0.1f, 1.0f}; ///< Dark wood color.

protected:
    void generate() override;
    glm::vec4 getPixel(float u, float v) override;

public:
    /**
     * @brief Constructor.
     */
    WoodTexture();

    /**
     * @brief Factory method to create a shared pointer to a WoodTexture instance.
     * @return std::shared_ptr<WoodTexture> New WoodTexture instance.
     */
    static std::shared_ptr<WoodTexture> create();

    /**
     * @brief Set the pattern scale.
     * @param scale Pattern scale.
     */
    void setScale(float scale);

    /**
     * @brief Set the number of rings.
     * @param rings Number of rings.
     */
    void setRings(float rings);

    /**
     * @brief Set the wood colors.
     * @param color1 Light color.
     * @param color2 Dark color.
     */
    void setColors(const glm::vec4 &color1, const glm::vec4 &color2);
};

/**
 * @typedef WoodTexturePtr
 * @brief Shared pointer type for WoodTexture.
 */
using WoodTexturePtr = std::shared_ptr<WoodTexture>;

} // namespace ivf
