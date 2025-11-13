#include <ivf/utils.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <glad/glad.h>

using namespace ivf;
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

float ivf::smoothStep(float edge0, float edge1, float x)
{
    float t = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

glm::mat4 ivf::rotationMatrix(const glm::vec3 &axis, float angle)
{
    return glm::rotate(glm::mat4(1.0f), angle, axis);
}

float ivf::calculateWeight(float distance, float start, float end, float falloff)
{
    if (distance < start)
        return 1.0f;
    if (distance > end)
        return 0.0f;

    float t = (distance - start) / (end - start);
    return std::pow(1.0f - t, falloff);
}

glm::mat4 ivf::createRotationMatrixTowards(glm::vec3 currentDirection, glm::vec3 targetDirection)
{
    // Normalize the vectors
    currentDirection = glm::normalize(currentDirection);
    targetDirection = glm::normalize(targetDirection);

    // Calculate the cross product and the angle
    glm::vec3 rotationAxis = glm::cross(currentDirection, targetDirection);
    float angle = acos(glm::clamp(glm::dot(currentDirection, targetDirection), -1.0f, 1.0f));

    // Create the rotation matrix
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, rotationAxis);

    return rotationMatrix;
}

void ivf::vectorToEuler(const glm::vec3 &unitVector, float &ax, float &ay)
{
    // Ensure the vector is normalized
    glm::vec3 dir = glm::normalize(unitVector);

    // Pitch (ax) is the angle between the vector and the xz-plane
    ax = std::atan2(dir.x, -dir.z);

    // Yaw (ay) is the angle between the vector's projection on the xz-plane and the z-axis
    ay = std::atan2(-dir.x, -dir.z);

    // Convert radians to degrees
    ax = glm::degrees(ax);
    ay = glm::degrees(ay);
}

float LinearInterpolFunc::tri(float t)
{
    if (abs(t) < 1)
        return 1.0 - abs(t);
    else
        return 0.0f;
}

LinearInterpolFunc::LinearInterpolFunc()
{}

void LinearInterpolFunc::addPoint(glm::vec3 p)
{
    m_points.push_back(p);
}

void ivf::LinearInterpolFunc::clear()
{
    m_points.clear();
}

int ivf::LinearInterpolFunc::size()
{
    return m_points.size();
}

glm::vec3 LinearInterpolFunc::operator()(float t)
{
    glm::vec3 pos(0.0, 0.0, 0.0);

    float tt = 0.0;

    if ((t > 0.0) && (t <= float(m_points.size() - 1)))
        tt = t * float(m_points.size() - 1);
    else if (t > float(m_points.size() - 1))
        tt = float(m_points.size() - 1);

    for (int i = 0; i < m_points.size(); i++)
        pos += m_points[i] * tri(tt - float(i));

    return pos;
}

void ivf::clearError()
{
    GLenum err = glGetError();
}

GLenum ivf::checkPrintError(const std::string context, const std::string file, const long line)
{
    GLenum err = glGetError();

    switch (err)
    {
    case GL_NO_ERROR:
        break;
    case GL_INVALID_ENUM:
        cout << file << ", line " << line << " - " << context
             << " - Error : An unacceptable value is specified for an enumerated argument. (GL_INVALID_ENUM)" << endl;
        break;
    case GL_INVALID_VALUE:
        cout << file << ", line " << line << " - " << context
             << " - Error: A numeric argument is out of range. (GL_INVALID_VALUE)" << endl;
        break;
    case GL_INVALID_OPERATION:
        cout << file << ", line " << line << " - " << context
             << " - Error: The specified operation is not allowed in the current state. (GL_INVALID_OPERATION)" << endl;
        break;
    /*
    case GL_STACK_OVERFLOW:
        cout << "Error: This command would cause a stack overflow. (GL_STACK_OVERFLOW)" << endl;
        break;
    case GL_STACK_UNDERFLOW:
        cout << "Error: This command would cause a stack underflow. (GL_STACK_UNDERFLOW)" << endl;
        break;
    case GL_TABLE_TOO_LARGE:
        cout << "Error: The specified table exceeds the implementation's maximum supported table size.
    (GL_TABLE_TOO_LARGE)" << endl; break;
    */
    case GL_OUT_OF_MEMORY:
        cout << file << ", line " << line << " - " << context
             << " - Error: There is not enough memory left to execute the command. (GL_OUT_OF_MEMORY)" << endl;
        break;
    default:
        cout << file << ", line " << line << " - " << context << " - Error: Unknown error code." << endl;
        break;
    }

    return err;
}

ivf::TransformManager *ivf::xfmMgr()
{
    return ivf::TransformManager::instance();
}

float ivf::random(float a, float b)
{
    double r = (double)rand() / (double)RAND_MAX;
    return a + (b - a) * float(r);
}

double ivf::random(double a, double b)
{
    double r = (double)rand() / (double)RAND_MAX;
    return a + (b - a) * r;
}

void ivf::randomSeed(unsigned int seed)
{
    srand(seed);
}

void ivf::randomSeed()
{
    srand((unsigned int)time(NULL));
}
