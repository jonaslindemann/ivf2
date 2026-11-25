#pragma once

/**
 * @file streamlines.h
 * @brief Streamlines of a vector field: tracing (GL-free), seed helpers and a scene node
 * drawing them as lines or tubes.
 */

#include <ivfvis/colormap.h>
#include <ivfvis/vector_field.h>

#include <ivf/bounding_box.h>
#include <ivf/mesh_node.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace ivfvis {

/** @brief Which way streamlines are traced from their seeds. */
enum class TraceDirection {
    Forward,  ///< Along the field.
    Backward, ///< Against the field.
    Both      ///< Both ways, joined at the seed (default).
};

/** @brief Settings of traceStreamline(). */
struct StreamlineOptions {
    float step{0.0f};             ///< Step length; 0 (default): 1/200 of the field's bounds diagonal (or 0.01).
    int maxSteps{2000};           ///< Most steps per direction.
    float maxLength{0.0f};        ///< Longest length per direction; 0: no limit.
    float minSpeed{1e-6f};        ///< Stop where the field is weaker than this.
    TraceDirection direction{TraceDirection::Both};
};

/** @brief A traced streamline: points in order along the field, and the field strength at each. */
struct Streamline {
    std::vector<glm::vec3> points;
    std::vector<float> speeds;

    /** @brief Length along the points. */
    [[nodiscard]] float length() const;
};

/**
 * @brief Trace the streamline through @p seed: steps of equal length along the field's
 * direction (4th order Runge-Kutta), until it leaves the field (VectorField::contains()),
 * reaches a weak spot, or the step or length limit.
 */
[[nodiscard]] Streamline traceStreamline(const VectorField &field, const glm::vec3 &seed,
                                         const StreamlineOptions &options = {});

/** @brief traceStreamline() for each seed (lines with fewer than 2 points are left out). */
[[nodiscard]] std::vector<Streamline> traceStreamlines(const VectorField &field, std::span<const glm::vec3> seeds,
                                                       const StreamlineOptions &options = {});

/** @brief @p count seeds evenly from @p a to @p b. */
[[nodiscard]] std::vector<glm::vec3> seedLine(const glm::vec3 &a, const glm::vec3 &b, int count);

/** @brief @p nu x @p nv seeds on the rectangle from @p origin spanned by @p u and @p v. */
[[nodiscard]] std::vector<glm::vec3> seedPlane(const glm::vec3 &origin, const glm::vec3 &u, const glm::vec3 &v,
                                               int nu, int nv);

/** @brief @p count seeds on a circle around @p center with normal @p normal. */
[[nodiscard]] std::vector<glm::vec3> seedCircle(const glm::vec3 &center, const glm::vec3 &normal, float radius,
                                                int count);

/** @brief @p count seeds at random in @p bounds (repeatable for a given @p seed). */
[[nodiscard]] std::vector<glm::vec3> seedRandom(const ivf::BoundingBox &bounds, int count, unsigned seed = 1);

/**
 * @class Streamlines
 * @brief Scene node tracing and drawing the streamlines of a VectorField from seed points.
 *
 * Lines are drawn unlit; tubes are lit meshes that cast shadows. With a colormap, lines and
 * tubes are coloured by the field strength along them (following changes to the colormap);
 * otherwise lines use setColor() and tubes the node's material.
 *
 * @code
 * auto lines = ivfvis::Streamlines::create(field, ivfvis::seedCircle({0, 0, 0}, {0, 1, 0}, 0.8f, 24));
 * lines->setTubes(true);
 * lines->setTubeRadius(0.02f);
 * lines->setColormap(cmap);
 * scene->add(lines);
 * @endcode
 */
class Streamlines : public ivf::MeshNode {
public:
    Streamlines(VectorFieldPtr field, std::vector<glm::vec3> seeds);

    [[nodiscard]] static std::shared_ptr<Streamlines> create(VectorFieldPtr field, std::vector<glm::vec3> seeds);

    void setField(VectorFieldPtr field);
    [[nodiscard]] VectorFieldPtr field() const;

    void setSeeds(std::vector<glm::vec3> seeds);
    [[nodiscard]] const std::vector<glm::vec3> &seeds() const;

    void setOptions(const StreamlineOptions &options);
    [[nodiscard]] const StreamlineOptions &options() const;

    /** @brief Draw tubes (true) or lines (false, default). */
    void setTubes(bool tubes);
    [[nodiscard]] bool tubes() const;

    /** @brief Tube radius (default 0.02). */
    void setTubeRadius(float radius);
    [[nodiscard]] float tubeRadius() const;

    /** @brief Corners around a tube (default 8). */
    void setTubeSides(int sides);

    /** @brief Colour by field strength (nullptr: setColor() for lines, the material for tubes). */
    void setColormap(ColormapPtr colormap);
    [[nodiscard]] ColormapPtr colormap() const;

    /** @brief Colour of lines without a colormap (default white). */
    void setColor(const glm::vec3 &color);

    /** @brief The traced lines. */
    [[nodiscard]] const std::vector<Streamline> &lines() const;

    /** @brief Range of the field strength along all lines (invalid if there are none). */
    [[nodiscard]] ValueRange speedRange() const;

    /** @brief Trace again (after the field's data changed). */
    void update();

    void collect(ivf::RenderQueue &queue, const ivf::CollectState &state) override;

private:
    void rebuild();

    VectorFieldPtr m_field;
    std::vector<glm::vec3> m_seeds;
    StreamlineOptions m_options;
    bool m_tubes{false};
    float m_tubeRadius{0.02f};
    int m_tubeSides{8};
    ColormapPtr m_colormap;
    glm::vec3 m_color{1.0f};

    std::vector<Streamline> m_lines;
    std::vector<float> m_vertexSpeed;
    std::shared_ptr<ivf::Material> m_lineMaterial;
    const Colormap *m_coloredWith{nullptr};
    std::uint64_t m_coloredVersion{0};
};

using StreamlinesPtr = std::shared_ptr<Streamlines>;

} // namespace ivfvis
