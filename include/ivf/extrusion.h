#pragma once

#include <ivf/mesh_node.h>

#include <ivf/extrusion_profile.h>
#include <ivf/path_frames.h>
#include <ivf/extrusion_builder.h>

#include <glm/glm.hpp>

#include <functional>
#include <vector>

namespace ivf {

/**
 * @class Extrusion
 * @brief Node that sweeps a 2D profile along a 3D path to produce a swept (extruded) mesh.
 *
 * Extrusion is the unified, GLE-style extrusion node for ivf2. A cross-section (@ref
 * ExtrusionProfile) is swept along a spine defined by control points. The spine can be treated
 * as a piecewise-linear polyline or smoothly interpolated, with configurable corner joins,
 * twist-free (rotation-minimizing) frames, end caps, normal/texture styles, and optional
 * per-station affine transforms (taper/twist/colour).
 *
 * The class builds an indexed triangle mesh via @ref ExtrusionBuilder. It inherits from MeshNode
 * for transformation and scene-graph integration, and exposes its main parameters through the
 * property inspection interface.
 */
class Extrusion : public MeshNode {
private:
    std::vector<glm::vec3> m_spinePoints; ///< Control points of the path.
    ExtrusionProfile m_profile;           ///< Cross-section profile.
    bool m_profileExplicit{false};        ///< True when a profile has been supplied explicitly.

    std::function<SectionTransform(float)> m_sectionFn; ///< Optional per-station transform.

    // Inspectable parameters (primitive-typed for the property system).
    double m_radius{0.25};   ///< Radius used for the default circular profile.
    int m_profileSegments{32}; ///< Segment count for the default circular profile.
    bool m_capStart{true};   ///< Generate a start cap.
    bool m_capEnd{true};     ///< Generate an end cap.
    bool m_closedSpine{false}; ///< Treat the spine as a closed loop.
    int m_spineInterp{0};    ///< 0 = Polyline, 1 = CatmullRom.
    int m_frameMethod{0};    ///< 0 = RotationMinimizing, 1 = Frenet, 2 = FixedUp.
    int m_joinStyle{1};      ///< 0 = Raw, 1 = Angle, 2 = Cut, 3 = Round.
    int m_normalStyle{0};    ///< 0 = Smooth, 1 = Facet.
    int m_texMode{0};        ///< 0 = Normalized, 1 = PerLength.
    int m_samples{64};       ///< Sample count for CatmullRom spines.
    int m_cornerSegments{6}; ///< Arc segments for Round joins.
    double m_cornerRadius{0.2}; ///< Corner radius for Cut/Round joins.
    glm::vec3 m_upVector{0.0f}; ///< Reference up vector (auto-chosen when zero).

public:
    Extrusion();

    static std::shared_ptr<Extrusion> create();

    /// @name Configuration
    /// @{

    /** @brief Set the cross-section profile. */
    void setProfile(const ExtrusionProfile &profile);
    const ExtrusionProfile &profile() const;

    /** @brief Set the spine control points (and optionally interpolation/sample count). */
    void setSpine(const std::vector<glm::vec3> &points, SpineInterp interp = SpineInterp::Polyline,
                  int samples = 64);

    void setClosedSpine(bool closed);
    void setCaps(bool start, bool end);
    void setJoinStyle(JoinStyle style);
    void setFrameMethod(FrameMethod method);
    void setSpineInterp(SpineInterp interp);
    void setNormalStyle(NormalStyle style);
    void setTexMode(TexMode mode);
    void setUpVector(const glm::vec3 &up);
    void setCornerRadius(double radius);
    void setCornerSegments(int segments);
    void setSamples(int samples);

    /** @brief Set a per-station affine transform (taper/twist/colour). t runs 0..1. */
    void setSectionTransform(std::function<SectionTransform(float)> fn);

    /// @}

    /// @name Legacy/compatibility API
    /// @{

    void addPathPoint(glm::vec3 p);
    void clearPath();
    void addSectionPoint(glm::vec2 p);
    void clearSection();
    void createCircleSection(float r, int segments = 36);

    /// @}

protected:
    virtual void doSetup() override;
    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;
};

/**
 * @typedef ExtrusionPtr
 * @brief Shared pointer type for Extrusion.
 */
typedef std::shared_ptr<Extrusion> ExtrusionPtr;

}; // namespace ivf
