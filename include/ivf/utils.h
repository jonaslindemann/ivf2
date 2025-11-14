#pragma once

/**
 * @file utils.h
 * @brief Utility functions and classes for math, OpenGL error handling, and animation in the ivf library.
 */

#include <glad/glad.h>
#include <string>
#include <source_location>
#include <ivf/transform_manager.h>

namespace ivf {

/**
 * @brief Smooth step function for falloff interpolation.
 * @param edge0 Lower edge of the transition.
 * @param edge1 Upper edge of the transition.
 * @param x Input value.
 * @return Interpolated value between 0 and 1.
 */
float smoothStep(float edge0, float edge1, float x);

/**
 * @brief Linear interpolation between two values.
 * @tparam T Type supporting arithmetic operations.
 * @param a Start value.
 * @param b End value.
 * @param t Interpolation factor (typically in [0, 1]).
 * @return Interpolated value.
 */
template <typename T> T lerp(const T &a, const T &b, float t)
{
    return a + t * (b - a);
}

/**
 * @brief Create a rotation matrix for rotation around an arbitrary axis.
 * @param axis Axis of rotation (should be normalized).
 * @param angle Angle of rotation in radians.
 * @return 4x4 rotation matrix.
 */
glm::mat4 rotationMatrix(const glm::vec3 &axis, float angle);

/**
 * @brief Calculate a weight based on distance and falloff.
 * @param distance Distance value.
 * @param start Start of the falloff region.
 * @param end End of the falloff region.
 * @param falloff Falloff exponent (default 1.0f).
 * @return Weight value between 0 and 1.
 */
float calculateWeight(float distance, float start, float end, float falloff = 1.0f);

/**
 * @class LinearInterpolFunc
 * @brief Piecewise linear interpolation function for animation or curve evaluation.
 *
 * Stores a sequence of 3D points and provides linear interpolation between them.
 */
class LinearInterpolFunc {
private:
    std::vector<glm::vec3> m_points; ///< Control points for interpolation.

    /**
     * @brief Triangle wave helper function.
     * @param t Input value.
     * @return Triangle wave value.
     */
    float tri(float t);

public:
    /**
     * @brief Default constructor.
     */
    LinearInterpolFunc();

    /**
     * @brief Add a control point.
     * @param p 3D point to add.
     */
    void addPoint(glm::vec3 p);

    /**
     * @brief Remove all control points.
     */
    void clear();

    /**
     * @brief Get the number of control points.
     * @return Number of points.
     */
    int size();

    /**
     * @brief Evaluate the interpolated value at parameter t.
     * @param t Parameter in [0, 1] or as needed.
     * @return Interpolated 3D value.
     */
    glm::vec3 operator()(float t);
};

/**
 * @brief Generate a random float in the range [a, b].
 * @param a Lower bound.
 * @param b Upper bound.
 * @return Random float value.
 */
float random(float a, float b);

/**
 * @brief Generate a random double in the range [a, b].
 * @param a Lower bound.
 * @param b Upper bound.
 * @return Random double value.
 */
double random(double a, double b);

void randomSeed(unsigned int seed);

void randomSeed();


/**
 * @brief Get the global TransformManager instance.
 * @return Pointer to the TransformManager.
 */
TransformManager *xfmMgr();

/**
 * @brief Create a rotation matrix that rotates one direction vector to another.
 * @param currentDirection The current direction vector.
 * @param targetDirection The target direction vector.
 * @return 4x4 rotation matrix.
 */
glm::mat4 createRotationMatrixTowards(glm::vec3 currentDirection, glm::vec3 targetDirection);

/**
 * @brief Convert a unit vector to Euler angles (azimuth and elevation).
 * @param unitVector Input unit vector.
 * @param ax Output azimuth angle (radians).
 * @param ay Output elevation angle (radians).
 */
void vectorToEuler(const glm::vec3 &unitVector, float &ax, float &ay);

// ============================================================================
// Modern C++20 Error Handling API
// ============================================================================

/**
 * @brief Clear all OpenGL errors.
 */
void clearError();

/**
 * @brief Check for OpenGL errors and print them with context information.
 * @param context Description of the operation or statement.
 * @param file Source file name (optional).
 * @param line Source line number (optional).
 * @return GLenum Error code (GL_NO_ERROR if none).
 */
GLenum checkPrintError(const std::string context, const std::string file = "", const long line = 0);

#ifdef _DEBUG

/**
 * @brief Execute a callable and check for OpenGL errors with automatic source location tracking.
 * @tparam F Callable type (lambda, function pointer, functor).
 * @param func Callable to execute.
 * @param location Source location (automatically captured).
 * 
 * @example
 * ivf::checkError([&]{ 
 *     glBindBuffer(GL_ARRAY_BUFFER, vbo);
 *     glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
 * });
 */
template <typename F>
inline void checkError(F&& func, const std::source_location& location = std::source_location::current()) {
    clearError();
    func();
    checkPrintError("OpenGL call", location.file_name(), location.line());
}

/**
 * @brief Begin an OpenGL error checking block.
 * 
 * @example
 * ivf::errorBegin();
 * glBindTexture(GL_TEXTURE_2D, texId);
 * glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 * ivf::errorEnd("Texture setup");
 */
inline void checkErrorBegin() {
    clearError();
}

/**
 * @brief End an OpenGL error checking block and report errors.
 * @param name Name or description of the block.
 * @param location Source location (automatically captured).
 */
inline void checkErrorEnd(const std::string& name, const std::source_location& location = std::source_location::current()) {
    checkPrintError(name, location.file_name(), location.line());
}

#else

// Release mode: inline no-op implementations
template <typename F>
inline void checkError(F&& func, const std::source_location& = std::source_location::current()) {
    func();
}

inline void checkErrorBegin() {}

inline void checkErrorEnd(const std::string&, const std::source_location& = std::source_location::current()) {}

#endif

}; // namespace ivf

// ============================================================================
// Legacy Macro Compatibility Layer
// ============================================================================

#ifdef _DEBUG

/**
 * @def GL_ERR(stmt)
 * @brief Legacy macro to execute an OpenGL statement and check for errors in debug builds.
 * @param stmt OpenGL statement to execute.
 * @deprecated Use ivf::checkError() with a lambda instead for better type safety.
 * 
 * @example
 * // Old (still works):
 * GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
 * 
 * // New (preferred):
 * ivf::checkError([&]{ glBindBuffer(GL_ARRAY_BUFFER, vbo); });
 */
#define GL_ERR(stmt)                                                           \
    ivf::clearError();                                                         \
    stmt;                                                                      \
    ivf::checkPrintError(#stmt, __FILE__, __LINE__);

/**
 * @def GL_ERR_BEGIN
 * @brief Legacy macro to clear OpenGL errors before a block of statements in debug builds.
 * @deprecated Use ivf::checkErrorBegin() instead.
 *
 * @example
 * // Old (still works):
 * GL_ERR_BEGIN
 * glBindBuffer(GL_ARRAY_BUFFER, vbo);
 * GL_ERR_END("Buffer setup");
 * 
 * // New (preferred):
 * ivf::errorBegin();
 * glBindBuffer(GL_ARRAY_BUFFER, vbo);
 * ivf::errorEnd("Buffer setup");
 */
#define GL_ERR_BEGIN ivf::checkErrorBegin();

/**
 * @def GL_ERR_END(name)
 * @brief Legacy macro to check and print OpenGL errors after a block of statements in debug builds.
 * @param name Name or description of the block.
 * @deprecated Use ivf::errorEnd() instead for automatic source location tracking.
 */
#define GL_ERR_END(name) ivf::checkErrorEnd(name);

#else

// Release mode: macros become no-ops
#define GL_ERR(stmt) stmt;
#define GL_ERR_BEGIN
#define GL_ERR_END(name)

#endif
