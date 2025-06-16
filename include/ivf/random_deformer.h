#pragma once

#include <ivf/deformer.h>
#include <glm/glm.hpp>
#include <functional>
#include <random>
#include <vector>
#include <memory>

namespace ivf {

/**
 * @class RandomDeformer
 * @brief Deformer that applies random or procedural noise-based deformation to mesh vertices.
 *
 * The RandomDeformer class supports multiple noise types (uniform, Gaussian, Perlin, Simplex, Cellular, Fractal)
 * and deformation modes (absolute, additive, directional, radial, surface normal). It provides
 * configuration for noise intensity, frequency, offset, direction, center, time, seed, and fractal parameters.
 * The deformer caches per-vertex noise for consistency and supports animated and procedural effects.
 */
class RandomDeformer : public Deformer {
public:
    /**
     * @enum NoiseType
     * @brief Types of noise supported for deformation.
     */
    enum class NoiseType {
        UNIFORM,  ///< Uniform random distribution
        GAUSSIAN, ///< Gaussian (normal) distribution
        PERLIN,   ///< Perlin noise
        SIMPLEX,  ///< Simplex noise
        CELLULAR, ///< Cellular/Worley noise
        FRACTAL   ///< Fractal noise (multiple octaves)
    };

    /**
     * @enum DeformationMode
     * @brief Modes for applying noise-based deformation.
     */
    enum class DeformationMode {
        ABSOLUTE,      ///< Set position to noisy value
        ADDITIVE,      ///< Add noise to original position
        DIRECTIONAL,   ///< Move along specified direction by noise amount
        RADIAL,        ///< Move radially from center by noise amount
        SURFACE_NORMAL ///< Move along surface normal by noise amount
    };

private:
    NoiseType m_noiseType;  ///< Current noise type.
    DeformationMode m_mode; ///< Current deformation mode.
    glm::vec3 m_intensity;  ///< Noise intensity per axis.
    glm::vec3 m_frequency;  ///< Noise frequency per axis.
    glm::vec3 m_offset;     ///< Noise offset (for animation).
    glm::vec3 m_direction;  ///< Direction for directional mode.
    glm::vec3 m_center;     ///< Center for radial mode.

    float m_time;        ///< Time for animated noise.
    unsigned int m_seed; ///< Random seed.
    int m_octaves;       ///< Number of octaves for fractal noise.
    float m_persistence; ///< Persistence for fractal noise.
    float m_lacunarity;  ///< Lacunarity for fractal noise.

    // Random number generation
    mutable std::mt19937 m_rng;
    mutable std::uniform_real_distribution<float> m_uniformDist;
    mutable std::normal_distribution<float> m_gaussianDist;

    // Per-vertex noise cache for consistency
    mutable std::vector<glm::vec3> m_noiseCache;
    mutable bool m_cacheValid;

public:
    /**
     * @brief Constructor.
     * @param noiseType Type of noise to use.
     * @param mode Deformation mode.
     */
    RandomDeformer(NoiseType noiseType = NoiseType::PERLIN, DeformationMode mode = DeformationMode::ADDITIVE);

    /**
     * @brief Factory method to create a shared pointer to a RandomDeformer instance.
     * @param noiseType Type of noise to use.
     * @param mode Deformation mode.
     * @return std::shared_ptr<RandomDeformer> New RandomDeformer instance.
     */
    static std::shared_ptr<RandomDeformer> create(NoiseType noiseType = NoiseType::PERLIN,
                                                  DeformationMode mode = DeformationMode::ADDITIVE);

    // Noise configuration
    /**
     * @brief Set the noise type.
     * @param type NoiseType value.
     */
    void setNoiseType(NoiseType type);
    /**
     * @brief Get the current noise type.
     * @return NoiseType value.
     */
    NoiseType noiseType() const;

    /**
     * @brief Set the deformation mode.
     * @param mode DeformationMode value.
     */
    void setDeformationMode(DeformationMode mode);
    /**
     * @brief Get the current deformation mode.
     * @return DeformationMode value.
     */
    DeformationMode deformationMode() const;

    /**
     * @brief Set the noise intensity per axis.
     * @param intensity Intensity vector.
     */
    void setIntensity(const glm::vec3 &intensity);
    /**
     * @brief Set a uniform noise intensity for all axes.
     * @param uniformIntensity Intensity value.
     */
    void setIntensity(float uniformIntensity);
    /**
     * @brief Get the current noise intensity.
     * @return glm::vec3 Intensity vector.
     */
    glm::vec3 intensity() const;

    /**
     * @brief Set the noise frequency per axis.
     * @param frequency Frequency vector.
     */
    void setFrequency(const glm::vec3 &frequency);
    /**
     * @brief Set a uniform noise frequency for all axes.
     * @param uniformFrequency Frequency value.
     */
    void setFrequency(float uniformFrequency);
    /**
     * @brief Get the current noise frequency.
     * @return glm::vec3 Frequency vector.
     */
    glm::vec3 frequency() const;

    /**
     * @brief Set the noise offset (for animation).
     * @param offset Offset vector.
     */
    void setOffset(const glm::vec3 &offset);
    /**
     * @brief Get the current noise offset.
     * @return glm::vec3 Offset vector.
     */
    glm::vec3 offset() const;

    /**
     * @brief Set the direction for directional deformation mode.
     * @param direction Direction vector.
     */
    void setDirection(const glm::vec3 &direction);
    /**
     * @brief Get the current direction for directional mode.
     * @return glm::vec3 Direction vector.
     */
    glm::vec3 direction() const;

    /**
     * @brief Set the center for radial deformation mode.
     * @param center Center position.
     */
    void setCenter(const glm::vec3 &center);
    /**
     * @brief Get the current center for radial mode.
     * @return glm::vec3 Center position.
     */
    glm::vec3 center() const;

    /**
     * @brief Set the time for animated noise.
     * @param time Time value.
     */
    void setTime(float time);
    /**
     * @brief Get the current time for animated noise.
     * @return float Time value.
     */
    float time() const;

    /**
     * @brief Set the random seed.
     * @param seed Seed value.
     */
    void setSeed(unsigned int seed);
    /**
     * @brief Get the current random seed.
     * @return unsigned int Seed value.
     */
    unsigned int seed() const;

    // Fractal noise parameters
    /**
     * @brief Set the number of octaves for fractal noise.
     * @param octaves Number of octaves.
     */
    void setOctaves(int octaves);
    /**
     * @brief Get the number of octaves for fractal noise.
     * @return int Number of octaves.
     */
    int octaves() const;

    /**
     * @brief Set the persistence for fractal noise.
     * @param persistence Persistence value.
     */
    void setPersistence(float persistence);
    /**
     * @brief Get the persistence for fractal noise.
     * @return float Persistence value.
     */
    float persistence() const;

    /**
     * @brief Set the lacunarity for fractal noise.
     * @param lacunarity Lacunarity value.
     */
    void setLacunarity(float lacunarity);
    /**
     * @brief Get the lacunarity for fractal noise.
     * @return float Lacunarity value.
     */
    float lacunarity() const;

    // Cache management
    /**
     * @brief Invalidate the per-vertex noise cache.
     */
    void invalidateCache();
    /**
     * @brief Regenerate the per-vertex noise cache.
     */
    void regenerateNoise();

    /**
     * @brief Apply the deformation to the input vertices.
     */
    virtual void apply() override;
    /**
     * @brief Create a copy of the deformer for animation keyframes.
     * @return Unique pointer to the cloned Deformer.
     */
    virtual std::unique_ptr<Deformer> clone() const override;

private:
    /**
     * @brief Generate noise for a given position and vertex index.
     * @param position Vertex position.
     * @param vertexIndex Index of the vertex.
     * @return glm::vec3 Noise vector.
     */
    glm::vec3 generateNoise(const glm::vec3 &position, GLuint vertexIndex) const;
    float perlinNoise(const glm::vec3 &position) const;
    float simplexNoise(const glm::vec3 &position) const;
    float cellularNoise(const glm::vec3 &position) const;
    float fractalNoise(const glm::vec3 &position) const;

    // Utility functions for noise generation
    float fade(float t) const;
    float lerp(float t, float a, float b) const;
    float grad(int hash, float x, float y, float z) const;
    glm::vec3 randomGradient(int ix, int iy, int iz) const;
};

/**
 * @typedef RandomDeformerPtr
 * @brief Shared pointer type for RandomDeformer.
 */
typedef std::shared_ptr<RandomDeformer> RandomDeformerPtr;

}; // namespace ivf
