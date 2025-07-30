#pragma once

#include <ivf/gl.h>
#include <ivf/property_inspectable.h>

namespace ivf {

/**
 * @class Effect
 * @brief Base class for post-processing and rendering effects.
 *
 * The Effect class provides a common interface for implementing post-processing effects
 * or custom rendering passes. It manages effect timing, resolution, and shader program usage.
 * Derived classes should override doLoad() and doUpdateParams() to implement effect-specific
 * resource loading and parameter updates.
 */
class Effect : public GLBase, public PropertyInspectable {
private:
    float m_time{0.0};  ///< Current effect time (for time-based effects).
    int m_width{0};     ///< Width of the effect's render target.
    int m_height{0};    ///< Height of the effect's render target.
    std::string m_name; ///< Name of the effect (optional).

public:
    /**
     * @brief Default constructor.
     */
    Effect();

    /**
     * @brief Virtual destructor.
     */
    virtual ~Effect();

    /**
     * @brief Factory method to create a shared pointer to an Effect instance.
     * @return std::shared_ptr<Effect> New Effect instance.
     */
    static std::shared_ptr<Effect> create();

    /**
     * @brief Set the current time for the effect (useful for animated effects).
     * @param time Time value.
     */
    void setTime(float time);

    /**
     * @brief Set the resolution for the effect's render target.
     * @param width Width in pixels.
     * @param height Height in pixels.
     */
    void setResolution(int width, int height);

    /**
     * @brief Set the name of the effect (for identification).
     * @param name Name of the effect.
     */
    void setName(const std::string &name);

    /**
     * @brief Get the name of the effect.
     * @return std::string Name of the effect.
     */
    std::string name() const;

    /**
     * @brief Get the current effect time.
     * @return float Current time value.
     */
    float time();

    /**
     * @brief Get the current render target width.
     * @return int Width in pixels.
     */
    int width();

    /**
     * @brief Get the current render target height.
     * @return int Height in pixels.
     */
    int height();

    /**
     * @brief Load and initialize the effect's resources (e.g., shaders).
     */
    void load();

    /**
     * @brief Activate the effect for rendering (binds the shader program, etc.).
     */
    void use();

    /**
     * @brief Get the shader program associated with this effect.
     * @return ProgramPtr Shared pointer to the shader program.
     */
    ProgramPtr program();

protected:
    ProgramPtr m_program; ///< Shader program used by the effect.

    /**
     * @brief Load effect-specific resources (to be overridden by derived classes).
     */
    virtual void doLoad(){};

    /**
     * @brief Update effect-specific parameters (to be overridden by derived classes).
     */
    virtual void doUpdateParams(){};
};

/**
 * @typedef EffectPtr
 * @brief Shared pointer type for Effect.
 */
typedef std::shared_ptr<Effect> EffectPtr;

} // namespace ivf
