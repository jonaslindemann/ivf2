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

class TransformManager {
private:
    enum class MatrixMode {
        MODEL,
        VIEW,
        PROJECTION
    };

    MatrixMode m_matrixMode;

    glm::vec3 m_viewPos;

    glm::mat4 m_modelMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    std::vector<glm::mat4> m_modelStack;
    std::vector<glm::mat4> m_projectionStack;
    std::vector<glm::mat4> m_viewStack;

    GLint m_modelId;
    GLint m_viewId;
    GLint m_projectionId;
    GLint m_viewPosId;

    TransformManager();
    static TransformManager *m_instance;

    void updateShaderMvpMatrix();

public:
    static TransformManager *instance()
    {
        if (!m_instance) {
            if (!m_instance)
                m_instance = new TransformManager();
        }
        return m_instance;
    }

    static TransformManager *create()
    {
        if (!m_instance) {
            if (!m_instance)
                m_instance = new TransformManager();
        }
        return m_instance;
    }

    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    // Generic functions

    void pushMatrix();
    void popMatrix();

    // Generic transforms

    void translate3f(float tx, float ty, float tz);
    void translate(glm::vec3 pos);
    void translate2f(float tx, float ty);
    void rotate(float rx, float ry, float rz, float angle);
    void rotateDeg(float rx, float ry, float rz, float angle);
    void rotateDeg(glm::vec3 axis, float angle);
    void rotate(float ax, float ay, float az);
    void rotateDeg(float ax, float ay, float az);
    void rotateToVector(glm::vec3 v);
    void scale(float sx, float sy, float sz);
    void multMatrix(glm::mat4 m);
    void alignWithAxisAngle(glm::vec3 axis, float angle);

    void identity();

    // Projection matrices

    void ortho2d(float left, float right, float bottom, float top);
    void perspective(float fovy, float aspect, float zNear, float zFar);

    // Viewing transforms

    void lookAt(float xe, float ye, float ze, float xc, float yc, float zc, float xu, float yu, float zu);

    void lookAt(double xe, double ye, double ze, double xc, double yc, double zc, double xu, double yu, double zu);

    void lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
    void lookAt(glm::vec3 eye, glm::vec3 center);

    void setMatrixMode(MatrixMode mode);
    MatrixMode matrixMode();

    void enableModelMatrix();
    void enableProjectionMatrix();
    void enableViewMatrix();
};

typedef TransformManager *TransformManagerPtr;

} // namespace ivf
