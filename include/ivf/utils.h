#pragma once

#include <glad/glad.h>

#include <string>

#include <ivf/transform_manager.h>

namespace ivf {

// Smooth step function for falloff
float smoothStep(float edge0, float edge1, float x);

// Linear interpolation for animation
template<typename T>
T lerp(const T& a, const T& b, float t) {
    return a + t * (b - a);
}

// Rotation matrix around arbitrary axis
glm::mat4 rotationMatrix(const glm::vec3& axis, float angle);

// Distance-based weight calculation
float calculateWeight(float distance, float start, float end, float falloff = 1.0f);    

class LinearInterpolFunc {
private:
    std::vector<glm::vec3> m_points;

    float tri(float t);

public:
    LinearInterpolFunc();

    void addPoint(glm::vec3 p);
    void clear();
    int size();

    glm::vec3 operator()(float t);
};

void clearError();
GLenum checkPrintError(const std::string context, const std::string file = "", const long line = 0);
float random(float a, float b);
double random(double a, double b);

TransformManager *xfmMgr();

glm::mat4 createRotationMatrixTowards(glm::vec3 currentDirection, glm::vec3 targetDirection);
void vectorToEuler(const glm::vec3 &unitVector, float &ax, float &ay);
}; // namespace ivf

#ifdef _DEBUG
#define GL_ERR(stmt)                                                                                                   \
    ivf::clearError();                                                                                                 \
    stmt;                                                                                                              \
    ivf::checkPrintError(#stmt, __FILE__, __LINE__);

#define GL_ERR_BEGIN ivf::clearError();
#define GL_ERR_END(name) ivf::checkPrintError(name, __FILE__, __LINE__);
#else
#define GL_ERR(stmt) stmt;
#define GL_ERR_BEGIN
#define GL_ERR_END(name)
#endif

