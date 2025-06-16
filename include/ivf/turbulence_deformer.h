#pragma once

/**
 * @file turbulence_deformer.h
 * @brief Declares the TurbulenceDeformer class for procedural mesh deformation using turbulence noise in the ivf
 * library.
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
 * @class TurbulenceDeformer
 * @brief Deformer that applies turbulence-based procedural noise to mesh vertices.
 *
 * The TurbulenceDeformer class generates animated, fractal-like turbulence effects by
 * displacing mesh vertices using layered noise. It supports control over scale, intensity,
 * number of octaves, persistence, animation speed, and random seed. Inherits from FunctionDeformer,
 * allowing integration with the ivf deformation and property inspection system.
 */
class TurbulenceDeformer : public FunctionDeformer {
private:
    float m_scale;          ///< Scale of the turbulence pattern.
    float m_intensity;      ///< Intensity (amplitude) of the displacement.
    int m_octaves;          ///< Number of noise octaves (layers).
    float m_persistence;    ///< Persistence (amplitude falloff per octave).
    float m_animationSpeed; ///< Speed of turbulence animation.
    unsigned int m_seed;    ///< Random seed for noise generation.

public:
    /**
     * @brief Construct a TurbulenceDeformer with the given parameters.
     * @param scale Scale of the turbulence pattern (default 1.0f).
     * @param intensity Intensity of the displacement (default 1.0f).
     * @param octaves Number of noise octaves (default 4).
     * @param persistence Amplitude falloff per octave (default 0.5f).
     * @param animationSpeed Speed of turbulence animation (default 1.0f).
     */
    TurbulenceDeformer(float scale = 1.0f, float intensity = 1.0f, int octaves = 4, float persistence = 0.5f,
                       float animationSpeed = 1.0f);

    /**
     * @brief Factory method to create a shared pointer to a TurbulenceDeformer instance.
     * @param scale Scale of the turbulence pattern (default 1.0f).
     * @param intensity Intensity of the displacement (default 1.0f).
     * @param octaves Number of noise octaves (default 4).
     * @param persistence Amplitude falloff per octave (default 0.5f).
     * @param animationSpeed Speed of turbulence animation (default 1.0f).
     * @return std::shared_ptr<TurbulenceDeformer> New TurbulenceDeformer instance.
     */
    static std::shared_ptr<TurbulenceDeformer> create(float scale = 1.0f, float intensity = 1.0f, int octaves = 4,
                                                      float persistence = 0.5f, float animationSpeed = 1.0f);

    /**
     * @brief Set the scale of the turbulence pattern.
     * @param scale Scale value.
     */
    void setScale(float scale);

    /**
     * @brief Get the current scale of the turbulence pattern.
     * @return float Scale value.
     */
    float scale() const;

    /**
     * @brief Set the intensity (amplitude) of the displacement.
     * @param intensity Intensity value.
     */
    void setIntensity(float intensity);

    /**
     * @brief Get the current intensity (amplitude) of the displacement.
     * @return float Intensity value.
     */
    float intensity() const;

    /**
     * @brief Set the number of noise octaves (layers).
     * @param octaves Number of octaves.
     */
    void setOctaves(int octaves);

    /**
     * @brief Get the number of noise octaves (layers).
     * @return int Number of octaves.
     */
    int octaves() const;

    /**
     * @brief Set the persistence (amplitude falloff per octave).
     * @param persistence Persistence value.
     */
    void setPersistence(float persistence);

    /**
     * @brief Get the persistence (amplitude falloff per octave).
     * @return float Persistence value.
     */
    float persistence() const;

    /**
     * @brief Set the animation speed of the turbulence.
     * @param speed Animation speed value.
     */
    void setAnimationSpeed(float speed);

    /**
     * @brief Get the animation speed of the turbulence.
     * @return float Animation speed value.
     */
    float animationSpeed() const;

    /**
     * @brief Set the random seed for noise generation.
     * @param seed Random seed value.
     */
    void setSeed(unsigned int seed);

    /**
     * @brief Get the random seed for noise generation.
     * @return unsigned int Random seed value.
     */
    unsigned int seed() const;

    /**
     * @brief Clone the deformer for animation keyframes or duplication.
     * @return std::unique_ptr<Deformer> Cloned TurbulenceDeformer instance.
     */
    virtual std::unique_ptr<Deformer> clone() const override;

private:
    /**
     * @brief Update the internal displacement function based on current parameters.
     */
    void updateFunction();

    /**
     * @brief Static helper to compute turbulence noise at a position.
     * @param position 3D position.
     * @param octaves Number of noise octaves.
     * @param persistence Amplitude falloff per octave.
     * @param seed Random seed.
     * @return float Turbulence value.
     */
    static float turbulence(const glm::vec3 &position, int octaves, float persistence, unsigned int seed);

protected:
    /**
     * @brief Register properties for inspection (editor integration).
     */
    virtual void setupProperties() override;
};

/**
 * @typedef TurbulenceDeformerPtr
 * @brief Shared pointer type for TurbulenceDeformer.
 */
typedef std::shared_ptr<TurbulenceDeformer> TurbulenceDeformerPtr;

} // namespace ivf
