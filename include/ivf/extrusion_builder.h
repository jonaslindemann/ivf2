#pragma once

#include <ivf/extrusion_profile.h>
#include <ivf/path_frames.h>

#include <glm/glm.hpp>

#include <functional>
#include <vector>

namespace ivf {

/**
 * @struct MeshData
 * @brief Plain CPU-side indexed mesh produced by the extrusion builder.
 *
 * Positions/normals/texCoords/colors are parallel arrays (one entry per vertex). Indices are
 * triangle index triples referencing those arrays. This is intentionally decoupled from any
 * OpenGL/scene-graph types so the builder stays a pure geometry generator.
 */
struct MeshData {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec4> colors;
    std::vector<glm::uvec3> indices;
};

/**
 * @brief Surface normal style for the swept walls.
 */
enum class NormalStyle {
    Smooth, ///< Per-vertex normals shared between adjacent faces (rounded look).
    Facet   ///< Per-face normals (flat look); profile points are duplicated per edge.
};

/**
 * @brief How the V texture coordinate is generated along the path.
 */
enum class TexMode {
    Normalized, ///< V runs 0..1 over the whole path.
    PerLength   ///< V equals cumulative arc length.
};

/**
 * @struct SectionTransform
 * @brief Per-station affine transform applied to the profile before placement.
 *
 * Drives tapering (scale), twisting (twist), and per-station colouring (color), in the spirit of
 * GLE's gleSuperExtrusion / twist extrusion.
 */
struct SectionTransform {
    glm::vec2 scale{1.0f, 1.0f}; ///< Profile scale in (x, y).
    float twist{0.0f};           ///< Rotation of the profile about the tangent, in radians.
    glm::vec4 color{1.0f};       ///< Vertex colour for this station.
};

/**
 * @struct ExtrusionOptions
 * @brief Options controlling how a profile is swept along a set of frames.
 */
struct ExtrusionOptions {
    bool capStart{true};                          ///< Generate a cap at the first frame.
    bool capEnd{true};                            ///< Generate a cap at the last frame.
    bool closedPath{false};                       ///< Connect the last frame back to the first.
    NormalStyle normalStyle{NormalStyle::Smooth}; ///< Wall normal style.
    TexMode texMode{TexMode::Normalized};         ///< V texture coordinate mode.
    std::function<SectionTransform(float)> sectionFn; ///< Optional per-station transform (t in [0,1]).
};

/**
 * @class ExtrusionBuilder
 * @brief Sweeps a 2D profile along a sequence of oriented frames into an indexed mesh.
 *
 * The builder is a pure geometry generator: given a profile, a list of path frames, and a set of
 * options it produces @ref MeshData (walls plus optional caps). It contains no OpenGL or scene
 * graph dependencies.
 */
class ExtrusionBuilder {
public:
    static MeshData build(const ExtrusionProfile &profile, const std::vector<PathFrame> &frames,
                          const ExtrusionOptions &options = {});
};

} // namespace ivf
