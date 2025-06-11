#pragma once

#include <ivf/deformer.h>
#include <ivf/function_deformer.h>
#include <glm/glm.hpp>
#include <functional>
#include <random>
#include <vector>
#include <memory>

namespace ivf {

class TurbulenceDeformer : public FunctionDeformer {
private:
    float m_scale;
    float m_intensity;
    int m_octaves;
    float m_persistence;
    float m_animationSpeed;
    unsigned int m_seed;
    
public:
    TurbulenceDeformer(float scale = 1.0f, float intensity = 1.0f, int octaves = 4,
                       float persistence = 0.5f, float animationSpeed = 1.0f);
    
    static std::shared_ptr<TurbulenceDeformer> create(
        float scale = 1.0f, float intensity = 1.0f, int octaves = 4,
        float persistence = 0.5f, float animationSpeed = 1.0f);
    
    void setScale(float scale);
    float scale() const;
    
    void setIntensity(float intensity);
    float intensity() const;
    
    void setOctaves(int octaves);
    int octaves() const;
    
    void setPersistence(float persistence);
    float persistence() const;
    
    void setAnimationSpeed(float speed);
    float animationSpeed() const;
    
    void setSeed(unsigned int seed);
    unsigned int seed() const;
    
    virtual std::unique_ptr<Deformer> clone() const override;
    
private:
    void updateFunction();
    static float turbulence(const glm::vec3& position, int octaves, float persistence, unsigned int seed);
};

typedef std::shared_ptr<TurbulenceDeformer> TurbulenceDeformerPtr;

}; // namespace ivf