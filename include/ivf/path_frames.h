#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace ivf {

/**
 * @struct PathFrame
 * @brief An oriented coordinate frame at a station (sample point) along a path.
 *
 * Each frame carries a position and an orthonormal basis (tangent, normal, binormal). The
 * cross-section of an extrusion is placed in the (normal, binormal) plane at each frame. The
 * @c v member stores the cumulative arc length along the path, used to generate the V texture
 * coordinate.
 */
struct PathFrame {
    glm::vec3 position{0.0f};            ///< Position of the frame in world space.
    glm::vec3 tangent{0.0f, 0.0f, 1.0f}; ///< Unit tangent (direction of travel).
    glm::vec3 normal{1.0f, 0.0f, 0.0f};  ///< Unit normal (profile local x-axis).
    glm::vec3 binormal{0.0f, 1.0f, 0.0f};///< Unit binormal (profile local y-axis).
    float v{0.0f};                       ///< Cumulative arc length along the path.
};

/**
 * @brief How control points are turned into a continuous spine.
 */
enum class SpineInterp {
    Polyline,  ///< Use the control points directly (piecewise linear).
    CatmullRom ///< Smoothly interpolate the control points with a cubic spline.
};

/**
 * @brief Method used to assign the rotational orientation of frames along the path.
 */
enum class FrameMethod {
    RotationMinimizing, ///< Twist-free frames (double-reflection method). Recommended.
    Frenet,             ///< Curvature-based frames (may flip at inflection points).
    FixedUp             ///< Keep the normal aligned to a fixed up vector where possible.
};

/**
 * @brief Corner treatment for piecewise-linear (Polyline) spines.
 *
 * Only meaningful for @c SpineInterp::Polyline. For closed polylines anything other than
 * @c Angle currently falls back to @c Angle.
 */
enum class JoinStyle {
    Raw,   ///< Sections kept perpendicular to each segment (sharp, may pinch).
    Angle, ///< Mitered: a single section in the corner's bisecting plane.
    Cut,   ///< Flat chamfer across the corner.
    Round  ///< Rounded corner formed by an arc of sections.
};

/**
 * @brief Sample a set of control points into a list of positions.
 * @param controlPoints Control points.
 * @param interp Polyline (returns the control points) or CatmullRom (samples a spline).
 * @param samples Number of samples for CatmullRom interpolation.
 */
std::vector<glm::vec3> sampleSpine(const std::vector<glm::vec3> &controlPoints, SpineInterp interp, int samples);

/**
 * @brief Build a complete list of oriented frames along a path defined by control points.
 *
 * This is the main entry point used by extrusion nodes. It samples/joins the control points
 * into stations (positions + tangents) and then assigns twist-free orientations.
 *
 * @param controlPoints Control points of the path.
 * @param interp Polyline or CatmullRom.
 * @param join Corner treatment (Polyline only).
 * @param samples Number of samples for CatmullRom.
 * @param frameMethod How to orient the frames.
 * @param upHint Reference up vector (auto-chosen when near zero).
 * @param closed Treat the path as a closed loop.
 * @param cornerRadius Corner radius used by Cut/Round joins.
 * @param cornerSegments Number of arc segments used by Round joins.
 */
std::vector<PathFrame> buildPathFrames(const std::vector<glm::vec3> &controlPoints,
                                       SpineInterp interp = SpineInterp::Polyline,
                                       JoinStyle join = JoinStyle::Angle, int samples = 64,
                                       FrameMethod frameMethod = FrameMethod::RotationMinimizing,
                                       const glm::vec3 &upHint = glm::vec3(0.0f), bool closed = false,
                                       float cornerRadius = 0.0f, int cornerSegments = 6);

/**
 * @brief Assign normals/binormals to frames that already have positions and tangents set.
 *
 * Implements the double-reflection rotation-minimizing frame propagation (Wang et al. 2008),
 * with optional closed-loop twist correction.
 */
void assignFrameOrientations(std::vector<PathFrame> &frames, FrameMethod frameMethod,
                             const glm::vec3 &upHint, bool closed);

} // namespace ivf
