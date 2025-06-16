#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace ivf {

/**
 * @class MatrixStack
 * @brief Utility class for managing a stack of transformation matrices.
 *
 * The MatrixStack class provides a stack-based interface for managing hierarchical
 * transformations, similar to the legacy OpenGL matrix stack. It supports push/pop,
 * identity, multiplication, translation, rotation, scaling, and retrieval of the
 * current (top) matrix or the global transformation.
 */
class MatrixStack {
private:
    std::vector<glm::mat4> m_stack; ///< Stack of transformation matrices.

public:
    /**
     * @brief Default constructor. Initializes the stack.
     */
    MatrixStack();

    /**
     * @brief Push the current matrix onto the stack.
     */
    void push();

    /**
     * @brief Load the identity matrix onto the top of the stack.
     */
    void loadIdentity();

    /**
     * @brief Pop the top matrix from the stack.
     */
    void pop();

    /**
     * @brief Multiply the top matrix by the given matrix.
     * @param m Matrix to multiply with the top matrix.
     */
    void multiply(glm::mat4 m);

    /**
     * @brief Apply a translation to the top matrix.
     * @param pos Translation vector.
     */
    void translate(glm::vec3 pos);

    /**
     * @brief Apply a rotation to the top matrix.
     * @param angle Rotation angle (in radians).
     * @param axis Axis of rotation.
     */
    void rotate(float angle, glm::vec3 axis);

    /**
     * @brief Apply a scaling transformation to the top matrix.
     * @param scale Scale vector.
     */
    void scale(glm::vec3 scale);

    /**
     * @brief Clear the matrix stack.
     */
    void clear();

    /**
     * @brief Get the number of matrices in the stack.
     * @return size_t Number of matrices.
     */
    size_t size();

    /**
     * @brief Get the top matrix on the stack.
     * @return glm::mat4 Top matrix.
     */
    glm::mat4 top();

    /**
     * @brief Get the global transformation matrix (top of the stack).
     * @return glm::mat4 Global transformation matrix.
     */
    glm::mat4 globalTransform();
};

} // namespace ivf
