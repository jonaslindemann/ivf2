#pragma once

#include <ivf/base.h>
#include <ivf/vertices.h>
#include <ivf/mesh.h>
#include <ivf/property_inspectable.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <functional>

namespace ivf {

// Forward declarations
class Mesh;
class MeshNode;

/**
 * @class Deformer
 * @brief Base class for mesh deformers.
 *
 * The Deformer class provides an interface for mesh deformation operations.
 * It manages original and deformed vertex data, supports enabling/disabling,
 * blending (weight), and property inspection for animation and editing.
 */
class Deformer : public Base, public PropertyInspectable {
protected:
    std::shared_ptr<Vertices> m_originalVertices; ///< Original (unmodified) vertices.
    std::shared_ptr<Vertices> m_deformedVertices; ///< Deformed (output) vertices.
    bool m_enabled;                               ///< Whether the deformer is enabled.
    float m_weight;                               ///< Blend weight for combining deformers.

public:
    /**
     * @brief Default constructor.
     */
    Deformer();

    /**
     * @brief Virtual destructor.
     */
    virtual ~Deformer() = default;

    /**
     * @brief Set the input vertices to be deformed.
     * @param vertices Shared pointer to the input Vertices.
     */
    virtual void setInput(std::shared_ptr<Vertices> vertices);

    /**
     * @brief Get the output (deformed) vertices.
     * @return Shared pointer to the deformed Vertices.
     */
    virtual std::shared_ptr<Vertices> getOutput();

    /**
     * @brief Apply the deformation to the input vertices.
     *
     * This is a pure virtual function that must be implemented by derived classes.
     */
    virtual void apply() = 0;

    /**
     * @brief Reset the deformer to its initial state.
     */
    virtual void reset();

    /**
     * @brief Enable or disable the deformer.
     * @param enabled True to enable, false to disable.
     */
    void setEnabled(bool enabled);

    /**
     * @brief Check if the deformer is enabled.
     * @return True if enabled, false otherwise.
     */
    bool enabled() const;

    /**
     * @brief Set the blend weight for this deformer.
     * @param weight Blend weight (typically in [0, 1]).
     */
    void setWeight(float weight);

    /**
     * @brief Get the blend weight for this deformer.
     * @return Blend weight.
     */
    float weight() const;

    /**
     * @brief Create a copy of the deformer for animation keyframes.
     * @return Unique pointer to the cloned Deformer.
     */
    virtual std::unique_ptr<Deformer> clone() const = 0;

protected:
    /**
     * @brief Register properties for inspection.
     */
    virtual void setupProperties() override;
};

}; // namespace ivf
