#pragma once

#include <ivf/deformer.h>
#include <glm/glm.hpp>
#include <functional>
#include <random>
#include <vector>
#include <memory>

namespace ivf {

/**
 * @class FunctionDeformer
 * @brief Deformer that applies a user-defined displacement function to mesh vertices.
 *
 * The FunctionDeformer class allows procedural deformation of mesh geometry by applying a
 * displacement function to each vertex. The function can be customized to produce effects
 * such as waves, ripples, turbulence, spirals, twirls, and gravity wells. Supports scaling,
 * offset, local/world space, and time-based animation.
 */
class FunctionDeformer : public Deformer {
public:
    /**
     * @brief Function signature for displacement: takes position, returns displacement vector.
     */
    using DisplacementFunction = std::function<glm::vec3(const glm::vec3 &)>;

private:
    DisplacementFunction m_displacementFunction; ///< Displacement function to apply.
    glm::vec3 m_scale;                           ///< Scale factor for displacement.
    glm::vec3 m_offset;                          ///< Offset added to input coordinates.
    bool m_useLocalSpace;                        ///< Whether to apply function in local or world space.
    float m_time;                                ///< Time parameter for animated functions.

public:
    /**
     * @brief Default constructor.
     */
    FunctionDeformer();

    /**
     * @brief Constructor with a custom displacement function.
     * @param func Displacement function to use.
     */
    FunctionDeformer(const DisplacementFunction &func);

    /**
     * @brief Factory method to create a shared pointer to a FunctionDeformer instance.
     * @return std::shared_ptr<FunctionDeformer> New FunctionDeformer instance.
     */
    static std::shared_ptr<FunctionDeformer> create();

    /**
     * @brief Factory method to create a shared pointer to a FunctionDeformer instance with a custom function.
     * @param func Displacement function to use.
     * @return std::shared_ptr<FunctionDeformer> New FunctionDeformer instance.
     */
    static std::shared_ptr<FunctionDeformer> create(const DisplacementFunction &func);

    /**
     * @brief Set the displacement function.
     * @param func Displacement function to use.
     */
    void setFunction(const DisplacementFunction &func);

    /**
     * @brief Set the scale factor for the displacement output.
     * @param scale Scale vector.
     */
    void setScale(const glm::vec3 &scale);

    /**
     * @brief Set a uniform scale factor for the displacement output.
     * @param uniformScale Uniform scale value.
     */
    void setScale(float uniformScale);

    /**
     * @brief Get the current scale factor.
     * @return glm::vec3 Scale vector.
     */
    glm::vec3 scale() const;

    /**
     * @brief Set the offset for the input coordinates.
     * @param offset Offset vector.
     */
    void setOffset(const glm::vec3 &offset);

    /**
     * @brief Get the current offset value.
     * @return glm::vec3 Offset vector.
     */
    glm::vec3 offset() const;

    /**
     * @brief Set whether to use local or world space for the function.
     * @param useLocal True for local space, false for world space.
     */
    void setUseLocalSpace(bool useLocal);

    /**
     * @brief Check if the function is applied in local space.
     * @return bool True if local space is used.
     */
    bool useLocalSpace() const;

    /**
     * @brief Set the time parameter for animated functions.
     * @param time Time value.
     */
    void setTime(float time);

    /**
     * @brief Get the current time parameter.
     * @return float Time value.
     */
    float time() const;

    /**
     * @brief Apply the displacement function to the mesh vertices.
     */
    virtual void apply() override;

    /**
     * @brief Clone the deformer (for animation keyframes, etc.).
     * @return std::unique_ptr<Deformer> Cloned deformer.
     */
    virtual std::unique_ptr<Deformer> clone() const override;

    // Predefined common displacement functions

    /**
     * @brief Create a sine wave displacement function.
     * @param frequency Frequency of the wave.
     * @param amplitude Amplitude of the wave.
     * @param direction Direction of the wave.
     * @return DisplacementFunction Sine wave function.
     */
    static DisplacementFunction createSineWave(float frequency = 1.0f, float amplitude = 1.0f,
                                               const glm::vec3 &direction = glm::vec3(0, 1, 0));

    /**
     * @brief Create a ripple displacement function.
     * @param frequency Frequency of the ripple.
     * @param amplitude Amplitude of the ripple.
     * @param center Center of the ripple.
     * @return DisplacementFunction Ripple function.
     */
    static DisplacementFunction createRipple(float frequency = 1.0f, float amplitude = 1.0f,
                                             const glm::vec3 &center = glm::vec3(0, 0, 0));

    /**
     * @brief Create a turbulence displacement function.
     * @param scale Scale of the turbulence.
     * @param intensity Intensity of the turbulence.
     * @param octaves Number of octaves for noise.
     * @return DisplacementFunction Turbulence function.
     */
    static DisplacementFunction createTurbulence(float scale = 1.0f, float intensity = 1.0f, int octaves = 4);

    /**
     * @brief Create a spiral displacement function.
     * @param pitch Pitch of the spiral.
     * @param radius Radius of the spiral.
     * @return DisplacementFunction Spiral function.
     */
    static DisplacementFunction createSpiral(float pitch = 1.0f, float radius = 1.0f);

    /**
     * @brief Create a twirl displacement function.
     * @param center Center of the twirl.
     * @param strength Strength of the twirl.
     * @param falloff Falloff of the twirl effect.
     * @return DisplacementFunction Twirl function.
     */
    static DisplacementFunction createTwirl(const glm::vec3 &center = glm::vec3(0, 0, 0), float strength = 1.0f,
                                            float falloff = 1.0f);

    /**
     * @brief Create a gravity well displacement function.
     * @param center Center of the gravity well.
     * @param strength Strength of the gravity well.
     * @param radius Radius of influence.
     * @return DisplacementFunction Gravity well function.
     */
    static DisplacementFunction createGravityWell(const glm::vec3 &center = glm::vec3(0, 0, 0), float strength = 1.0f,
                                                  float radius = 5.0f);

protected:
    virtual void setupProperties() override;
};

/**
 * @typedef FunctionDeformerPtr
 * @brief Shared pointer type for FunctionDeformer.
 */
typedef std::shared_ptr<FunctionDeformer> FunctionDeformerPtr;

}; // namespace ivf
