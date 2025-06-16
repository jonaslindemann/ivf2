#pragma once

#include <ivf/deformer.h>

namespace ivf {

/**
 * @class DeformerStack
 * @brief Manages a stack of deformers and applies them in sequence to mesh vertices.
 *
 * The DeformerStack class allows multiple Deformer objects to be chained together, applying
 * each deformer in order to a set of input vertices. This enables complex deformation effects
 * by combining simple deformers. The stack supports adding, removing, clearing, and accessing
 * deformers, as well as setting input vertices and retrieving the final output.
 */
class DeformerStack : public Base {
private:
    std::vector<std::shared_ptr<Deformer>> m_deformers; ///< List of deformers in the stack.
    std::shared_ptr<Vertices> m_inputVertices;          ///< Input vertices for deformation.
    std::shared_ptr<Vertices> m_outputVertices;         ///< Output vertices after deformation.

public:
    /**
     * @brief Default constructor.
     */
    DeformerStack();

    /**
     * @brief Factory method to create a shared pointer to a DeformerStack instance.
     * @return std::shared_ptr<DeformerStack> New DeformerStack instance.
     */
    static std::shared_ptr<DeformerStack> create();

    /**
     * @brief Add a deformer to the stack.
     * @param deformer Shared pointer to the deformer to add.
     */
    void addDeformer(std::shared_ptr<Deformer> deformer);

    /**
     * @brief Remove a deformer from the stack by pointer.
     * @param deformer Shared pointer to the deformer to remove.
     */
    void removeDeformer(std::shared_ptr<Deformer> deformer);

    /**
     * @brief Remove a deformer from the stack by index.
     * @param index Index of the deformer to remove.
     */
    void removeDeformer(size_t index);

    /**
     * @brief Remove all deformers from the stack.
     */
    void clear();

    /**
     * @brief Set the input vertices for the stack.
     * @param vertices Shared pointer to the input Vertices.
     */
    void setInput(std::shared_ptr<Vertices> vertices);

    /**
     * @brief Get the output vertices after all deformers have been applied.
     * @return std::shared_ptr<Vertices> Output vertices.
     */
    std::shared_ptr<Vertices> getOutput();

    /**
     * @brief Apply all deformers in sequence to the input vertices.
     */
    void apply(); // Apply all deformers in sequence

    /**
     * @brief Reset all deformers in the stack to their initial state.
     */
    void reset(); // Reset all deformers

    /**
     * @brief Get the number of deformers in the stack.
     * @return size_t Number of deformers.
     */
    size_t count() const;

    /**
     * @brief Get a deformer by index.
     * @param index Index of the deformer.
     * @return std::shared_ptr<Deformer> The deformer at the specified index.
     */
    std::shared_ptr<Deformer> deformer(size_t index);

    // Animation support

    /**
     * @brief Enable or disable all deformers in the stack.
     * @param enabled True to enable, false to disable.
     */
    void setEnabled(bool enabled);

    /**
     * @brief Set the blend weight for all deformers in the stack.
     * @param weight Blend weight (typically in [0, 1]).
     */
    void setWeight(float weight);
};

}; // namespace ivf
