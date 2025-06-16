#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>
#include <ivf/smartptr.h>

#include <string>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ivf {

/**
 * @class TransformManager
 * @brief Singleton class for managing model, view, and projection matrices and their stacks.
 *
 * The TransformManager class provides a global interface for managing the current
 * model, view, and projection matrices, as well as their stacks for hierarchical
 * transformations. It supports OpenGL-style matrix stack operations, transformation
 * utilities, and updates shader uniforms as needed.
 */
class TransformManager {
private:
    /**
     * @enum MatrixMode
     * @brief Specifies which matrix stack is currently active.
     */
    enum class MatrixMode {
        MODEL,     ///< Model matrix stack.
        VIEW,      ///< View matrix stack.
        PROJECTION ///< Projection matrix stack.
    };

    MatrixMode m_matrixMode; ///< Current matrix mode.

    glm::vec3 m_viewPos; ///< Camera/view position.

    glm::mat4 m_modelMatrix;      ///< Current model matrix.
    glm::mat4 m_viewMatrix;       ///< Current view matrix.
    glm::mat4 m_projectionMatrix; ///< Current projection matrix.

    std::vector<glm::mat4> m_modelStack;      ///< Stack for model matrices.
    std::vector<glm::mat4> m_projectionStack; ///< Stack for projection matrices.
    std::vector<glm::mat4> m_viewStack;       ///< Stack for view matrices.

    GLint m_modelId;      ///< Shader uniform location for model matrix.
    GLint m_viewId;       ///< Shader uniform location for view matrix.
    GLint m_projectionId; ///< Shader uniform location for projection matrix.
    GLint m_viewPosId;    ///< Shader uniform location for view position.

    TransformManager();                  ///< Private constructor for singleton pattern.
    static TransformManager *m_instance; ///< Singleton instance pointer.

    /**
     * @brief Update the shader with the current model, view, and projection matrices.
     */
    void updateShaderMvpMatrix();

public:
    /**
     * @brief Get the singleton instance of the TransformManager.
     * @return TransformManager* Pointer to the singleton instance.
     */
    static TransformManager *instance()
    {
        if (!m_instance)
        {
            if (!m_instance)
                m_instance = new TransformManager();
        }
        return m_instance;
    }

    /**
     * @brief Create the singleton instance of the TransformManager (if not already created).
     * @return TransformManager* Pointer to the singleton instance.
     */
    static TransformManager *create()
    {
        if (!m_instance)
        {
            if (!m_instance)
                m_instance = new TransformManager();
        }
        return m_instance;
    }

    /**
     * @brief Destroy the singleton instance and release all resources.
     */
    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    // Generic functions

    /**
     * @brief Push the current matrix onto the active stack.
     */
    void pushMatrix();

    /**
     * @brief Pop the top matrix from the active stack.
     */
    void popMatrix();

    // Generic transforms

    /**
     * @brief Apply a translation to the current matrix.
     * @param tx Translation along X.
     * @param ty Translation along Y.
     * @param tz Translation along Z.
     */
    void translate3f(float tx, float ty, float tz);

    /**
     * @brief Apply a translation to the current matrix.
     * @param pos Translation vector.
     */
    void translate(glm::vec3 pos);

    /**
     * @brief Apply a 2D translation to the current matrix.
     * @param tx Translation along X.
     * @param ty Translation along Y.
     */
    void translate2f(float tx, float ty);

    /**
     * @brief Apply a rotation to the current matrix (axis-angle, radians).
     * @param rx X component of axis.
     * @param ry Y component of axis.
     * @param rz Z component of axis.
     * @param angle Angle in radians.
     */
    void rotate(float rx, float ry, float rz, float angle);

    /**
     * @brief Apply a rotation to the current matrix (axis-angle, degrees).
     * @param rx X component of axis.
     * @param ry Y component of axis.
     * @param rz Z component of axis.
     * @param angle Angle in degrees.
     */
    void rotateDeg(float rx, float ry, float rz, float angle);

    /**
     * @brief Apply a rotation to the current matrix (axis-angle, degrees).
     * @param axis Rotation axis.
     * @param angle Angle in degrees.
     */
    void rotateDeg(glm::vec3 axis, float angle);

    /**
     * @brief Apply a rotation to the current matrix (Euler angles, radians).
     * @param ax Rotation around X.
     * @param ay Rotation around Y.
     * @param az Rotation around Z.
     */
    void rotate(float ax, float ay, float az);

    /**
     * @brief Apply a rotation to the current matrix (Euler angles, degrees).
     * @param ax Rotation around X.
     * @param ay Rotation around Y.
     * @param az Rotation around Z.
     */
    void rotateDeg(float ax, float ay, float az);

    /**
     * @brief Rotate the current matrix to align with a given vector.
     * @param v Target vector.
     */
    void rotateToVector(glm::vec3 v);

    /**
     * @brief Apply a scaling transformation to the current matrix.
     * @param sx Scale along X.
     * @param sy Scale along Y.
     * @param sz Scale along Z.
     */
    void scale(float sx, float sy, float sz);

    /**
     * @brief Multiply the current matrix by another matrix.
     * @param m Matrix to multiply with.
     */
    void multMatrix(glm::mat4 m);

    /**
     * @brief Align the current matrix with a given axis and angle.
     * @param axis Axis to align with.
     * @param angle Angle in radians.
     */
    void alignWithAxisAngle(glm::vec3 axis, float angle);

    /**
     * @brief Set the current matrix to the identity matrix.
     */
    void identity();

    // Projection matrices

    /**
     * @brief Set up a 2D orthographic projection matrix.
     * @param left Left clipping plane.
     * @param right Right clipping plane.
     * @param bottom Bottom clipping plane.
     * @param top Top clipping plane.
     */
    void ortho2d(float left, float right, float bottom, float top);

    /**
     * @brief Set up a perspective projection matrix.
     * @param fovy Field of view in the y direction (radians).
     * @param aspect Aspect ratio.
     * @param zNear Near clipping plane.
     * @param zFar Far clipping plane.
     */
    void perspective(float fovy, float aspect, float zNear, float zFar);

    // Viewing transforms

    /**
     * @brief Set up a view matrix using eye, center, and up vectors (float version).
     * @param xe Eye X.
     * @param ye Eye Y.
     * @param ze Eye Z.
     * @param xc Center X.
     * @param yc Center Y.
     * @param zc Center Z.
     * @param xu Up X.
     * @param yu Up Y.
     * @param zu Up Z.
     */
    void lookAt(float xe, float ye, float ze, float xc, float yc, float zc, float xu, float yu, float zu);

    /**
     * @brief Set up a view matrix using eye, center, and up vectors (double version).
     * @param xe Eye X.
     * @param ye Eye Y.
     * @param ze Eye Z.
     * @param xc Center X.
     * @param yc Center Y.
     * @param zc Center Z.
     * @param xu Up X.
     * @param yu Up Y.
     * @param zu Up Z.
     */
    void lookAt(double xe, double ye, double ze, double xc, double yc, double zc, double xu, double yu, double zu);

    /**
     * @brief Set up a view matrix using glm::vec3 for eye, center, and up.
     * @param eye Eye position.
     * @param center Center position.
     * @param up Up vector.
     */
    void lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

    /**
     * @brief Set up a view matrix using glm::vec3 for eye and center, with default up vector.
     * @param eye Eye position.
     * @param center Center position.
     */
    void lookAt(glm::vec3 eye, glm::vec3 center);

    /**
     * @brief Set the current matrix mode (MODEL, VIEW, PROJECTION).
     * @param mode Matrix mode.
     */
    void setMatrixMode(MatrixMode mode);

    /**
     * @brief Get the current matrix mode.
     * @return MatrixMode Current matrix mode.
     */
    MatrixMode matrixMode();

    /**
     * @brief Enable the model matrix for subsequent operations.
     */
    void enableModelMatrix();

    /**
     * @brief Enable the projection matrix for subsequent operations.
     */
    void enableProjectionMatrix();

    /**
     * @brief Enable the view matrix for subsequent operations.
     */
    void enableViewMatrix();

    /**
     * @brief Get a reference to the current view matrix.
     * @return glm::mat4& Reference to the view matrix.
     */
    glm::mat4 &viewMatrix();
};

/**
 * @typedef TransformManagerPtr
 * @brief Pointer type for TransformManager singleton.
 */
typedef TransformManager *TransformManagerPtr;

} // namespace ivf
