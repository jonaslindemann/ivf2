#pragma once

#include <ivf/deformer.h>
#include <glm/glm.hpp>
#include <functional>
#include <random>
#include <vector>
#include <memory>

namespace ivf {

class RandomDeformer : public Deformer {
public:
    enum class NoiseType {
        UNIFORM,        // Uniform random distribution
        GAUSSIAN,       // Gaussian (normal) distribution
        PERLIN,         // Perlin noise
        SIMPLEX,        // Simplex noise
        CELLULAR,       // Cellular/Worley noise
        FRACTAL         // Fractal noise (multiple octaves)
    };
    
    enum class DeformationMode {
        ABSOLUTE,       // Set position to noisy value
        ADDITIVE,       // Add noise to original position
        DIRECTIONAL,    // Move along specified direction by noise amount
        RADIAL,         // Move radially from center by noise amount
        SURFACE_NORMAL  // Move along surface normal by noise amount
    };
    
private:
    NoiseType m_noiseType;
    DeformationMode m_mode;
    glm::vec3 m_intensity;          // Noise intensity per axis
    glm::vec3 m_frequency;          // Noise frequency per axis
    glm::vec3 m_offset;             // Noise offset (for animation)
    glm::vec3 m_direction;          // Direction for directional mode
    glm::vec3 m_center;             // Center for radial mode
    
    float m_time;                   // Time for animated noise
    unsigned int m_seed;            // Random seed
    int m_octaves;                  // For fractal noise
    float m_persistence;            // For fractal noise
    float m_lacunarity;             // For fractal noise
    
    // Random number generation
    mutable std::mt19937 m_rng;
    mutable std::uniform_real_distribution<float> m_uniformDist;
    mutable std::normal_distribution<float> m_gaussianDist;
    
    // Per-vertex noise cache for consistency
    mutable std::vector<glm::vec3> m_noiseCache;
    mutable bool m_cacheValid;
    
public:
    RandomDeformer(NoiseType noiseType = NoiseType::PERLIN, 
                   DeformationMode mode = DeformationMode::ADDITIVE);
    
    static std::shared_ptr<RandomDeformer> create(
        NoiseType noiseType = NoiseType::PERLIN,
        DeformationMode mode = DeformationMode::ADDITIVE);
    
    // Noise configuration
    void setNoiseType(NoiseType type);
    NoiseType noiseType() const;
    
    void setDeformationMode(DeformationMode mode);
    DeformationMode deformationMode() const;
    
    void setIntensity(const glm::vec3& intensity);
    void setIntensity(float uniformIntensity);
    glm::vec3 intensity() const;
    
    void setFrequency(const glm::vec3& frequency);
    void setFrequency(float uniformFrequency);
    glm::vec3 frequency() const;
    
    void setOffset(const glm::vec3& offset);
    glm::vec3 offset() const;
    
    void setDirection(const glm::vec3& direction);
    glm::vec3 direction() const;
    
    void setCenter(const glm::vec3& center);
    glm::vec3 center() const;
    
    void setTime(float time);
    float time() const;
    
    void setSeed(unsigned int seed);
    unsigned int seed() const;
    
    // Fractal noise parameters
    void setOctaves(int octaves);
    int octaves() const;
    
    void setPersistence(float persistence);
    float persistence() const;
    
    void setLacunarity(float lacunarity);
    float lacunarity() const;
    
    // Cache management
    void invalidateCache();
    void regenerateNoise();
    
    virtual void apply() override;
    virtual std::unique_ptr<Deformer> clone() const override;
    
private:
    glm::vec3 generateNoise(const glm::vec3& position, GLuint vertexIndex) const;
    float perlinNoise(const glm::vec3& position) const;
    float simplexNoise(const glm::vec3& position) const;
    float cellularNoise(const glm::vec3& position) const;
    float fractalNoise(const glm::vec3& position) const;
    
    // Utility functions for noise generation
    float fade(float t) const;
    float lerp(float t, float a, float b) const;
    float grad(int hash, float x, float y, float z) const;
    glm::vec3 randomGradient(int ix, int iy, int iz) const;
};

typedef std::shared_ptr<RandomDeformer> RandomDeformerPtr;

}; // namespace ivf