#pragma once

/**
 * @file gpu_procedural_texture.h
 * @brief GPU-based procedural texture generation using render-to-texture.
 */

#include <ivf/texture.h>
#include <ivf/program.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

namespace ivf {

/**
 * @class GPUProceduralTexture
 * @brief Base class for GPU-generated procedural textures.
 *
 * This class uses render-to-texture (RTT) to generate procedural patterns
 * using fragment shaders. The generated texture can be used like any normal
 * texture in the rendering pipeline.
 */
class GPUProceduralTexture : public Texture {
protected:
    int m_width{512};                      ///< Texture width in pixels.
    int m_height{512};                     ///< Texture height in pixels.
    GLuint m_fbo{0};                       ///< Framebuffer object for rendering.
    GLuint m_quadVAO{0};                   ///< Vertex array object for fullscreen quad.
    GLuint m_quadVBO{0};                   ///< Vertex buffer object for quad vertices.
    GLuint m_quadEBO{0};                   ///< Element buffer object for quad indices.
    
    ProgramPtr m_generatorShader;          ///< Shader program for procedural generation.
    bool m_needsRegeneration{true};        ///< Flag indicating regeneration is needed.
    float m_time{0.0f};                    ///< Time value for animated effects.
    
    /**
     * @brief Create a fullscreen quad for rendering.
     */
    void createQuad();
    
    /**
     * @brief Cleanup OpenGL resources.
     */
    void cleanup();

public:
    /**
     * @brief Constructor.
     */
    GPUProceduralTexture();
    
    /**
     * @brief Destructor.
     */
    virtual ~GPUProceduralTexture();
    
    /**
     * @brief Set texture size.
     * @param width Texture width.
     * @param height Texture height.
     */
    void setSize(int width, int height);
    
    /**
     * @brief Get texture width.
     * @return int Texture width.
     */
    int width() const { return m_width; }
    
    /**
     * @brief Get texture height.
     * @return int Texture height.
     */
    int height() const { return m_height; }
    
    /**
     * @brief Mark texture for regeneration.
     * @param flag True to mark for regeneration.
     */
    void setNeedsRegeneration(bool flag = true);
    
    /**
     * @brief Generate the texture by rendering with the procedural shader.
     */
    virtual void regenerate();
    
    /**
     * @brief Set the generator shader.
     * @param shader Shader program to use for generation.
     */
    void setGeneratorShader(ProgramPtr shader);
    
    /**
     * @brief Set the time value for animated effects.
     * @param time Time value in seconds.
     */
    void setTime(float time);
    
    /**
     * @brief Get the current time value.
     * @return float Current time value.
     */
    float time() const { return m_time; }
    
protected:
    /**
     * @brief Set shader uniforms specific to the procedural type.
     * Override in derived classes to set custom uniforms.
     */
    virtual void setShaderUniforms() {}
};

/**
 * @class GPUCheckerboardTexture
 * @brief GPU-generated checkerboard pattern texture.
 */
class GPUCheckerboardTexture : public GPUProceduralTexture {
private:
    glm::vec4 m_color1{1.0f, 1.0f, 1.0f, 1.0f};  ///< First checker color.
    glm::vec4 m_color2{0.0f, 0.0f, 0.0f, 1.0f};  ///< Second checker color.
    float m_checkerSize{8.0f};                    ///< Number of checkers per axis.
    
protected:
    void setShaderUniforms() override;
    
public:
    /**
     * @brief Constructor.
     */
    GPUCheckerboardTexture();
    
    /**
     * @brief Factory method to create a shared pointer.
     * @return std::shared_ptr<GPUCheckerboardTexture> New instance.
     */
    static std::shared_ptr<GPUCheckerboardTexture> create();
    
    /**
     * @brief Set the checker colors.
     * @param color1 First color.
     * @param color2 Second color.
     */
    void setColors(const glm::vec4 &color1, const glm::vec4 &color2);
    
    /**
     * @brief Set the checker size.
     * @param size Number of checkers per axis.
     */
    void setCheckerSize(float size);
};

/**
 * @class GPUNoiseTexture
 * @brief GPU-generated Perlin noise texture.
 */
class GPUNoiseTexture : public GPUProceduralTexture {
private:
    glm::vec4 m_colorLow{0.0f, 0.0f, 0.0f, 1.0f};   ///< Color for low noise values.
    glm::vec4 m_colorHigh{1.0f, 1.0f, 1.0f, 1.0f};  ///< Color for high noise values.
    float m_scale{4.0f};                             ///< Noise scale.
    int m_octaves{4};                                ///< Number of octaves.
    float m_persistence{0.5f};                       ///< Amplitude decay per octave.
    float m_lacunarity{2.0f};                        ///< Frequency increase per octave.
    
protected:
    void setShaderUniforms() override;
    
public:
    /**
     * @brief Constructor.
     */
    GPUNoiseTexture();
    
    /**
     * @brief Factory method to create a shared pointer.
     * @return std::shared_ptr<GPUNoiseTexture> New instance.
     */
    static std::shared_ptr<GPUNoiseTexture> create();
    
    /**
     * @brief Set noise scale.
     * @param scale Noise scale (higher = more zoomed out).
     */
    void setScale(float scale);
    
    /**
     * @brief Set number of octaves.
     * @param octaves Number of octaves.
     */
    void setOctaves(int octaves);
    
    /**
     * @brief Set persistence.
     * @param persistence Amplitude decay (typically 0.5).
     */
    void setPersistence(float persistence);
    
    /**
     * @brief Set lacunarity.
     * @param lacunarity Frequency increase (typically 2.0).
     */
    void setLacunarity(float lacunarity);
    
    /**
     * @brief Set color range.
     * @param colorLow Color for low values.
     * @param colorHigh Color for high values.
     */
    void setColorRange(const glm::vec4 &colorLow, const glm::vec4 &colorHigh);
};

/**
 * @class GPUGradientTexture
 * @brief GPU-generated gradient texture.
 */
class GPUGradientTexture : public GPUProceduralTexture {
public:
    enum class Type { Linear, Radial };
    
private:
    Type m_type{Type::Linear};                      ///< Gradient type.
    glm::vec4 m_colorStart{0.0f, 0.0f, 0.0f, 1.0f}; ///< Start color.
    glm::vec4 m_colorEnd{1.0f, 1.0f, 1.0f, 1.0f};   ///< End color.
    float m_angle{0.0f};                             ///< Angle in radians (linear).
    glm::vec2 m_center{0.5f, 0.5f};                  ///< Center point (radial).
    
protected:
    void setShaderUniforms() override;
    
public:
    /**
     * @brief Constructor.
     */
    GPUGradientTexture();
    
    /**
     * @brief Factory method to create a shared pointer.
     * @return std::shared_ptr<GPUGradientTexture> New instance.
     */
    static std::shared_ptr<GPUGradientTexture> create();
    
    /**
     * @brief Set gradient type.
     * @param type Gradient type (Linear or Radial).
     */
    void setType(Type type);
    
    /**
     * @brief Set gradient colors.
     * @param colorStart Start color.
     * @param colorEnd End color.
     */
    void setColors(const glm::vec4 &colorStart, const glm::vec4 &colorEnd);
    
    /**
     * @brief Set gradient angle (for linear gradients).
     * @param angle Angle in radians.
     */
    void setAngle(float angle);
    
    /**
     * @brief Set center point (for radial gradients).
     * @param center Center point in normalized coordinates [0, 1].
     */
    void setCenter(const glm::vec2 &center);
};

/**
 * @typedef GPUProceduralTexturePtr
 * @brief Shared pointer type for GPUProceduralTexture.
 */
using GPUProceduralTexturePtr = std::shared_ptr<GPUProceduralTexture>;

/**
 * @typedef GPUCheckerboardTexturePtr
 * @brief Shared pointer type for GPUCheckerboardTexture.
 */
using GPUCheckerboardTexturePtr = std::shared_ptr<GPUCheckerboardTexture>;

/**
 * @typedef GPUNoiseTexturePtr
 * @brief Shared pointer type for GPUNoiseTexture.
 */
using GPUNoiseTexturePtr = std::shared_ptr<GPUNoiseTexture>;

/**
 * @typedef GPUGradientTexturePtr
 * @brief Shared pointer type for GPUGradientTexture.
 */
using GPUGradientTexturePtr = std::shared_ptr<GPUGradientTexture>;

} // namespace ivf
