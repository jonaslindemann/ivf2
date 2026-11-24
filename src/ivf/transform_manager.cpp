#include <ivf/transform_manager.h>
#include <ivf/render_context.h>

#include <glm/gtx/string_cast.hpp>

#include <ivf/gl_debug.h>
#include <ivf/shader_manager.h>
#include <ivf/utils.h>

#include <cstring>
#include <iostream>

using namespace ivf;
using namespace std;

// The manager lives in the current RenderContext (see ivf/render_context.h).
TransformManager *TransformManager::instance()
{
    return RenderContext::currentOrFallback()->transforms();
}

TransformManager *TransformManager::create()
{
    return instance();
}

void TransformManager::drop()
{
    // Managers are released together with their RenderContext.
}

TransformManager::TransformManager()
    : m_modelMatrix(1.0), m_viewMatrix(1.0), m_projectionMatrix(1.0), m_modelId(-1), m_viewId(-1), m_projectionId(-1),
      m_viewPosId(-1)
{
    m_modelId = ShaderManager::instance()->currentProgram()->uniformLoc("model");
    m_viewId = ShaderManager::instance()->currentProgram()->uniformLoc("view");
    m_projectionId = ShaderManager::instance()->currentProgram()->uniformLoc("projection");
    m_viewPosId = ShaderManager::instance()->currentProgram()->uniformLoc("viewPos");

    updateShaderMvpMatrix();
}

TransformManager::~TransformManager()
{
    if (m_cameraBuffer != 0)
        glDeleteBuffers(1, &m_cameraBuffer);
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

glm::mat4 &ivf::TransformManager::viewMatrix()
{
    return m_viewMatrix;
}

glm::mat4 &ivf::TransformManager::modelMatrix()
{
    return m_modelMatrix;
}

glm::mat4 &ivf::TransformManager::projectionMatrix()
{
    return m_projectionMatrix;
}

// function to display 4x4 matrix
void display_4x4(string tag, glm::mat4 m4)
{
    cout << tag << '\n';
    for (int col = 0; col < 4; ++col)
    {
        cout << "| ";
        for (int row = 0; row < 4; ++row)
        {
            cout << m4[row][col] << '\t';
        }
        cout << '\n';
    }
    cout << '\n';
}

void TransformManager::updateShaderMvpMatrix()
{
    // Plain uniforms for programs without the IvfCamera block (their locations are -1 otherwise).
    auto program = ShaderManager::instance()->currentProgram();
    program->uniformMatrix4(m_modelId, m_modelMatrix);
    program->uniformMatrix4(m_viewId, m_viewMatrix);
    program->uniformMatrix4(m_projectionId, m_projectionMatrix);
    program->uniformVec3(m_viewPosId, m_viewPos);
    syncCameraBlock();
}

void TransformManager::uploadView()
{
    auto program = ShaderManager::instance()->currentProgram();
    program->uniformMatrix4(m_viewId, m_viewMatrix);
    program->uniformVec3(m_viewPosId, m_viewPos);
    syncCameraBlock();
}

void TransformManager::uploadProjection()
{
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_projectionId, m_projectionMatrix);
    syncCameraBlock();
}

void TransformManager::syncCameraBlock()
{
    CameraBlock block;
    block.view = m_viewMatrix;
    block.projection = m_projectionMatrix;
    block.viewProjection = m_projectionMatrix * m_viewMatrix;
    block.viewPos = glm::vec4(m_viewPos, 1.0f);

    if (m_cameraBlockValid && std::memcmp(&block, &m_cameraBlock, sizeof(CameraBlock)) == 0)
        return; // unchanged: no upload

    if (m_cameraBuffer == 0)
    {
        if (glGenBuffers == nullptr)
            return; // no OpenGL functions loaded (GL-free tests)
        glGenBuffers(1, &m_cameraBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, m_cameraBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_DYNAMIC_DRAW);
        glLabel(GL_BUFFER, m_cameraBuffer, CameraBlockName);
    }
    else
        glBindBuffer(GL_UNIFORM_BUFFER, m_cameraBuffer);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraBlock), &block);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, CameraBlockBinding, m_cameraBuffer);

    m_cameraBlock = block;
    m_cameraBlockValid = true;
    if (auto *context = RenderContext::current())
        context->countCameraBlockUpload();
}

const CameraBlock &TransformManager::cameraBlock() const
{
    return m_cameraBlock;
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
    if (m_matrixMode == MatrixMode::MODEL)
    {
        if (m_modelStack.size() != 0)
        {
            m_modelMatrix = m_modelStack.back();
            m_modelStack.pop_back();
            ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
        }
    }
    else if (m_matrixMode == MatrixMode::PROJECTION)
    {
        if (m_projectionStack.size() != 0)
        {
            m_projectionMatrix = m_projectionStack.back();
            m_projectionStack.pop_back();
            uploadProjection();
        }
    }
    else
    {
        if (m_viewStack.size() != 0)
        {
            m_viewMatrix = m_viewStack.back();
            m_viewStack.pop_back();
            uploadView();
        }
    }
}

glm::mat4 TransformManager::translationMatrix(glm::vec3 t)
{
    return glm::translate(glm::mat4(1.0), t);
}

glm::mat4 TransformManager::eulerRotationMatrixDeg(float ax, float ay, float az)
{
    glm::mat4 m = glm::rotate(glm::mat4(1.0), glm::radians(ax), glm::vec3(1.0, 0.0, 0.0));
    m = m * glm::rotate(glm::mat4(1.0), glm::radians(ay), glm::vec3(0.0, 1.0, 0.0));
    m = m * glm::rotate(glm::mat4(1.0), glm::radians(az), glm::vec3(0.0, 0.0, 1.0));
    return m;
}

glm::mat4 TransformManager::axisRotationMatrixDeg(glm::vec3 axis, float angle)
{
    return glm::rotate(glm::mat4(1.0), glm::radians(angle), axis);
}

bool TransformManager::rotationToVectorMatrix(glm::vec3 v, glm::mat4 &result)
{
    // Rotate the local +X axis onto v (the shortest rotation).
    const float len = glm::length(v);
    if (len < 1e-6f)
        return false;

    const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
    const glm::vec3 dir = v / len;
    const glm::vec3 axis = glm::cross(xAxis, dir);
    const float cosAngle = glm::clamp(glm::dot(xAxis, dir), -1.0f, 1.0f);

    glm::mat4 m(1.0f);
    if (glm::length(axis) > 1e-6f)
        m = glm::rotate(m, std::acos(cosAngle), glm::normalize(axis));
    else if (cosAngle < 0.0f)
        m = glm::rotate(m, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)); // v points along -X
    result = m;
    return true;
}

glm::mat4 TransformManager::scaleMatrix(glm::vec3 s)
{
    return glm::scale(glm::mat4(1.0), s);
}

void TransformManager::setModelMatrix(const glm::mat4 &m)
{
    m_modelMatrix = m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void TransformManager::translate3f(float tx, float ty, float tz)
{
    glm::mat4 m = translationMatrix(glm::vec3(tx, ty, tz));
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

void ivf::TransformManager::rotateDeg(float rx, float ry, float rz, float angle)
{
    glm::mat4 m = axisRotationMatrixDeg(glm::vec3(rx, ry, rz), angle);
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::rotateDeg(glm::vec3 axis, float angle)
{
    glm::mat4 m = glm::rotate(glm::mat4(1.0), glm::radians(angle), axis);
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::rotate(float ax, float ay, float az)
{
    glm::mat4 m = glm::rotate(glm::mat4(1.0), ax, glm::vec3(1.0, 0.0, 0.0));
    m = m * glm::rotate(glm::mat4(1.0), ay, glm::vec3(0.0, 1.0, 0.0));
    m = m * glm::rotate(glm::mat4(1.0), az, glm::vec3(0.0, 0.0, 1.0));
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::rotateDeg(float ax, float ay, float az)
{
    glm::mat4 m = eulerRotationMatrixDeg(ax, ay, az);
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::rotateToVector(glm::vec3 v)
{
    glm::mat4 m(1.0f);
    if (!rotationToVectorMatrix(v, m))
        return;

    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::scale(float sx, float sy, float sz)
{
    glm::mat4 m = scaleMatrix(glm::vec3(sx, sy, sz));
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::multMatrix(glm::mat4 m)
{
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void ivf::TransformManager::alignWithAxisAngle(glm::vec3 axis, float angle)
{
    glm::mat4 m = glm::rotate(glm::mat4(1.0), angle, axis);
    m_modelMatrix = m_modelMatrix * m;
    ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
}

void TransformManager::lookAt(float xe, float ye, float ze, float xc, float yc, float zc, float xu, float yu, float zu)
{
    glm::mat4 m = glm::lookAt(glm::vec3(xe, ye, ze), glm::vec3(xc, yc, zc), glm::vec3(xu, yu, zu));
    m_viewMatrix = m_viewMatrix * m;
    uploadView();
}

void TransformManager::lookAt(double xe, double ye, double ze, double xc, double yc, double zc, double xu, double yu,
                              double zu)
{
    glm::mat4 m = glm::lookAt(glm::vec3(xe, ye, ze), glm::vec3(xc, yc, zc), glm::vec3(xu, yu, zu));
    m_viewMatrix = m_viewMatrix * m;
    uploadView();
}

void ivf::TransformManager::lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
    glm::mat4 m = glm::lookAt(eye, center, up);
    m_viewMatrix = m_viewMatrix * m;
    m_viewPos = eye;
    uploadView();
}

void ivf::TransformManager::lookAt(glm::vec3 eye, glm::vec3 center)
{
    glm::mat4 m = glm::lookAt(eye, center, glm::vec3(0.0, 1.0, 0.0));
    m_viewMatrix = m_viewMatrix * m;
    m_viewPos = eye;
    uploadView();
}

void ivf::TransformManager::setMatrixMode(MatrixMode mode)
{
    m_matrixMode = mode;
}

TransformManager::MatrixMode ivf::TransformManager::matrixMode() const
{
    return m_matrixMode;
}

void TransformManager::ortho2d(float left, float right, float bottom, float top)
{
    glm::mat4 m = glm::ortho(left, right, bottom, top);
    m_projectionMatrix = m_projectionMatrix * m;
    uploadProjection();
}

void TransformManager::perspective(float fovy, float aspect, float zNear, float zFar)
{
    glm::mat4 m = glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
    m_projectionMatrix = m_projectionMatrix * m;
    uploadProjection();
}

void TransformManager::identity()
{
    glm::mat4 m(1.0);

    if (m_matrixMode == MatrixMode::MODEL)
    {
        m_modelMatrix = m;
        ShaderManager::instance()->currentProgram()->uniformMatrix4(m_modelId, m_modelMatrix);
    }
    else if (m_matrixMode == MatrixMode::PROJECTION)
    {
        m_projectionMatrix = m;
        uploadProjection();
    }
    else
    {
        m_viewMatrix = m;
        uploadView();
    }
}

void TransformManager::refreshForProgram()
{
    auto prog = ShaderManager::instance()->currentProgram();
    m_modelId = prog->uniformLoc("model");
    m_viewId = prog->uniformLoc("view");
    m_projectionId = prog->uniformLoc("projection");
    m_viewPosId = prog->uniformLoc("viewPos");
    updateShaderMvpMatrix();
}

glm::vec3 TransformManager::viewPos() const
{
    return m_viewPos;
}

void TransformManager::setViewPos(glm::vec3 pos)
{
    m_viewPos = pos;
    syncCameraBlock();
}
