#pragma once

/**
 * @file wave_deformer.h
 * @brief Declares the WaveDeformer class for procedural mesh deformation using wave functions in the ivf library.
 */

#include <ivf/deformer.h>
#include <ivf/function_deformer.h>
#include <glm/glm.hpp>
#include <functional>
#include <random>
#include <vector>
#include <memory>

namespace ivf {

/**
 * @class WaveDeformer
 * @brief Deformer that applies a wave-based procedural displacement to mesh vertices.
 *
 * The WaveDeformer class generates animated wave effects by displacing mesh vertices
 * using configurable amplitude, frequency, speed, direction, and wave vector. Inherits from
 * FunctionDeformer, allowing integration with the ivf deformation and property inspection system.
 */
class WaveDeformer : public FunctionDeformer {
private:
    float m_amplitude;      ///< Amplitude of the wave.
    float m_frequency;      ///< Frequency of the wave.
    float m_speed;          ///< Speed of wave propagation.
    glm::vec3 m_direction;  ///< Direction of wave travel.
    glm::vec3 m_waveVector; ///< Wave vector for advanced control.

public:
    /**
     * @brief Construct a WaveDeformer with the given parameters.
     * @param amplitude Amplitude of the wave (default 1.0f).
     * @param frequency Frequency of the wave (default 1.0f).
     * @param speed Speed of wave propagation (default 1.0f).
     * @param direction Direction of wave travel (default (1, 0, 0)).
     * @param waveVector Wave vector (default (0, 1, 0)).
     */
    WaveDeformer(float amplitude = 1.0f, float frequency = 1.0f, float speed = 1.0f,
                 const glm::vec3 &direction = glm::vec3(1, 0, 0), const glm::vec3 &waveVector = glm::vec3(0, 1, 0));

    /**
     * @brief Factory method to create a shared pointer to a WaveDeformer instance.
     * @param amplitude Amplitude of the wave (default 1.0f).
     * @param frequency Frequency of the wave (default 1.0f).
     * @param speed Speed of wave propagation (default 1.0f).
     * @param direction Direction of wave travel (default (1, 0, 0)).
     * @param waveVector Wave vector (default (0, 1, 0)).
     * @return std::shared_ptr<WaveDeformer> New WaveDeformer instance.
     */
    static std::shared_ptr<WaveDeformer> create(float amplitude = 1.0f, float frequency = 1.0f, float speed = 1.0f,
                                                const glm::vec3 &direction = glm::vec3(1, 0, 0),
                                                const glm::vec3 &waveVector = glm::vec3(0, 1, 0));

    /**
     * @brief Set the amplitude of the wave.
     * @param amplitude Amplitude value.
     */
    void setAmplitude(float amplitude);

    /**
     * @brief Get the current amplitude of the wave.
     * @return float Amplitude value.
     */
    float amplitude() const;

    /**
     * @brief Set the frequency of the wave.
     * @param frequency Frequency value.
     */
    void setFrequency(float frequency);

    /**
     * @brief Get the current frequency of the wave.
     * @return float Frequency value.
     */
    float frequency() const;

    /**
     * @brief Set the speed of wave propagation.
     * @param speed Speed value.
     */
    void setSpeed(float speed);

    /**
     * @brief Get the current speed of wave propagation.
     * @return float Speed value.
     */
    float speed() const;

    /**
     * @brief Set the direction of wave travel.
     * @param direction Direction vector.
     */
    void setDirection(const glm::vec3 &direction);

    /**
     * @brief Get the current direction of wave travel.
     * @return glm::vec3 Direction vector.
     */
    glm::vec3 direction() const;

    /**
     * @brief Set the wave vector for advanced control.
     * @param waveVector Wave vector.
     */
    void setWaveVector(const glm::vec3 &waveVector);

    /**
     * @brief Get the current wave vector.
     * @return glm::vec3 Wave vector.
     */
    glm::vec3 waveVector() const;

    /**
     * @brief Clone the deformer for animation keyframes or duplication.
     * @return std::unique_ptr<Deformer> Cloned WaveDeformer instance.
     */
    virtual std::unique_ptr<Deformer> clone() const override;

private:
    /**
     * @brief Update the internal displacement function based on current parameters.
     */
    void updateFunction();

protected:
    /**
     * @brief Register properties for inspection (editor integration).
     */
    virtual void setupProperties() override;
};

/**
 * @typedef WaveDeformerPtr
 * @brief Shared pointer type for WaveDeformer.
 */
typedef std::shared_ptr<WaveDeformer> WaveDeformerPtr;

}; // namespace ivf
