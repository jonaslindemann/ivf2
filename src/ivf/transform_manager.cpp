#include <ivf/transform_manager.h>

#include <glm/gtx/string_cast.hpp>

#include <ivf/shader_manager.h>
#include <ivf/utils.h>

#include <iostream>

using namespace ivf;
using namespace std;

TransformManager *TransformManager::m_instance = 0;

TransformManager::TransformManager()
    : m_modelMatrix(1.0), m_projectionMatrix(1.0), m_viewMatrix(1.0), m_modelId(-1), m_viewId(-1), m_viewPosId(-1),
      m_projectionId(-1)
{
    m_modelId = ShaderManager::instance()->currentProgram()->uniformLoc("model");
    m_viewId = ShaderManager::instance()->currentProgram()->uniformLoc("view");
    m_projectionId = ShaderManager::instance()->currentProgram()->uniformLoc("projection");
    m_viewPosId = ShaderManager::instance()->currentProgram()->uniformLoc("viewPos");

    updateShaderMvpMatrix();
}

void TransformManager::enableModelMatrix()
{
    m_matrixMode = MatrixMode::MODEL;
}

void TransformManager::enableProjectionMatrix()
{
    m_matrixMode = MatrixMode::PROJECTION;
}

void TransformManager::enableViewMatrix()
{
    m_matrixMode = MatrixMode::VIEW;
}

// function to display 4x4 matrix
void display_4x4(string tag, glm::mat4 m4)
{
    cout << tag << '\n';
    for (int col = 0; col < 4; ++col) {
        cout << "| ";
        for (int row = 0; row < 4; ++row) {
            cout << m4[row][col] << '\t';
        }
        cout << '\n';
    }
    cout << '\n';
}

void TransformManager::updateShaderMvpMatrix()
{
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_viewId, m_viewMatrix);
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_projectionId, m_projectionMatrix);
    ShaderManager::instance()->currentProgram()->uniformVec3(m_viewPosId, m_viewPos);
}

void TransformManager::pushMatrix()
{
    if (m_matrixMode == MatrixMode::MODEL)
        m_modelStack.push_back(m_modelMatrix);
    else if (m_matrixMode == MatrixMode::PROJECTION)
        m_projectionStack.push_back(m_projectionMatrix);
    else
        m_viewStack.push_back(m_viewMatrix);
}

void TransformManager::popMatrix()
{
    if (m_matrixMode == MatrixMode::MODEL) {
        if (m_modelStack.size() != 0) {
            m_modelMatrix = m_modelStack.back();
            m_modelStack.pop_back();
            ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
        }
    }
    else if (m_matrixMode == MatrixMode::PROJECTION) {
        if (m_projectionStack.size() != 0) {
            m_projectionMatrix = m_projectionStack.back();
            m_projectionStack.pop_back();
            ShaderManager::instance()->currentProgram()->uniformMatrix4(m_projectionId, m_modelMatrix);
        }
    }
    else {
        if (m_viewStack.size() != 0) {
            m_viewMatrix = m_viewStack.back();
            m_viewStack.pop_back();
            ShaderManager::instance()->currentProgram()->uniformMatrix4(m_viewId, m_modelMatrix);
        }
    }
}

void TransformManager::translate3f(float tx, float ty, float tz)
{
    glm::mat4 m = glm::translate(glm::mat4(1.0), glm::vec3(tx, ty, tz));
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::translate(glm::vec3 pos)
{
    this->translate3f(pos.x, pos.y, pos.z);
}

void TransformManager::translate2f(float tx, float ty)
{
    glm::mat4 m = glm::translate(glm::mat4(1.0), glm::vec3(tx, ty, 0.0));
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void TransformManager::rotate(float rx, float ry, float rz, float angle)
{
    glm::mat4 m = glm::rotate(glm::mat4(1.0), angle, glm::vec3(rx, ry, rz));
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::scale(float sx, float sy, float sz)
{
    glm::mat4 m = glm::scale(glm::mat4(1.0), glm::vec3(sx, sy, sz));
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::multMatrix(glm::mat4 m)
{
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void TransformManager::lookAt(float xe, float ye, float ze, float xc, float yc, float zc, float xu, float yu, float zu)
{
    glm::mat4 m = glm::lookAt(glm::vec3(xe, ye, ze), glm::vec3(xc, yc, zc), glm::vec3(xu, yu, zu));
    m_viewMatrix = m_viewMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_viewId, m_viewMatrix);
}

void TransformManager::lookAt(double xe, double ye, double ze, double xc, double yc, double zc, double xu, double yu,
                              double zu)
{
    glm::mat4 m = glm::lookAt(glm::vec3(xe, ye, ze), glm::vec3(xc, yc, zc), glm::vec3(xu, yu, zu));
    m_viewMatrix = m_viewMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_viewId, m_viewMatrix);
}

void ivf::TransformManager::lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
    glm::mat4 m = glm::lookAt(eye, center, up);
    m_viewMatrix = m_viewMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_viewId, m_viewMatrix);
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_viewPosId, m_projectionMatrix);
}

void ivf::TransformManager::lookAt(glm::vec3 eye, glm::vec3 center)
{
    glm::mat4 m = glm::lookAt(eye, center, glm::vec3(0.0, 1.0, 0.0));
    m_viewMatrix = m_viewMatrix * m;
    m_viewPos = eye;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_viewId, m_viewMatrix);
    ShaderManager::instance()->currentProgram()->uniformVec3(m_viewPosId, m_viewPos);
}

void ivf::TransformManager::setMatrixMode(MatrixMode mode)
{
    m_matrixMode = mode;
}

TransformManager::MatrixMode ivf::TransformManager::matrixMode()
{
    return m_matrixMode;
}

void TransformManager::ortho2d(float left, float right, float bottom, float top)
{
    glm::mat4 m = glm::ortho(left, right, bottom, top);
    m_projectionMatrix = m_projectionMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_projectionId, m_projectionMatrix);
}

void TransformManager::perspective(float fovy, float aspect, float zNear, float zFar)
{
    glm::mat4 m = glm::perspective(fovy, aspect, zNear, zFar);
    m_projectionMatrix = m_projectionMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_projectionId, m_projectionMatrix);
}

void TransformManager::identity()
{
    glm::mat4 m(1.0);

    if (m_matrixMode == MatrixMode::MODEL) {
        m_modelMatrix = m;
        ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
    }
    else if (m_matrixMode == MatrixMode::PROJECTION) {
        m_projectionMatrix = m;
        ShaderManager::instance()->currentProgram()->uniformMatrix4(m_projectionId, m_projectionMatrix);
    }
    else {
        m_viewMatrix = m;
        ShaderManager::instance()->currentProgram()->uniformMatrix4(m_viewId, m_viewMatrix);
    }
}
