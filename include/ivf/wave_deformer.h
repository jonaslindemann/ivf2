#pragma once

#include <ivf/deformer.h>
#include <ivf/function_deformer.h>
#include <glm/glm.hpp>
#include <functional>
#include <random>
#include <vector>
#include <memory>

namespace ivf {

class WaveDeformer : public FunctionDeformer {
private:
    float m_amplitude;
    float m_frequency;
    float m_speed;
    glm::vec3 m_direction;
    glm::vec3 m_waveVector;
    
public:
    WaveDeformer(float amplitude = 1.0f, float frequency = 1.0f, float speed = 1.0f,
                 const glm::vec3& direction = glm::vec3(1, 0, 0),
                 const glm::vec3& waveVector = glm::vec3(0, 1, 0));
    
    static std::shared_ptr<WaveDeformer> create(
        float amplitude = 1.0f, float frequency = 1.0f, float speed = 1.0f,
        const glm::vec3& direction = glm::vec3(1, 0, 0),
        const glm::vec3& waveVector = glm::vec3(0, 1, 0));
    
    void setAmplitude(float amplitude);
    float amplitude() const;
    
    void setFrequency(float frequency);
    float frequency() const;
    
    void setSpeed(float speed);
    float speed() const;
    
    void setDirection(const glm::vec3& direction);
    glm::vec3 direction() const;
    
    void setWaveVector(const glm::vec3& waveVector);
    glm::vec3 waveVector() const;
    
    virtual std::unique_ptr<Deformer> clone() const override;
    
private:
    void updateFunction();
};

typedef std::shared_ptr<WaveDeformer> WaveDeformerPtr;

}; // namespace ivf

