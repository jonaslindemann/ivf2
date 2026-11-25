#pragma once

/**
 * @file glyphs.h
 * @brief Arrows showing a vector field: oriented along the vectors, scaled and coloured by
 * their length.
 */

#include <ivfvis/colormap.h>
#include <ivfvis/uniform_grid.h>
#include <ivfvis/vector_field.h>

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace ivfvis {

/** @brief Shape of the glyphs. */
enum class GlyphShape {
    Arrow, ///< Shaft and head (default).
    Cone,  ///< A cone pointing along the vector.
    Line   ///< A line from the tail to the tip (cheapest; unlit).
};

/** @brief How glyph lengths follow the vectors. */
enum class GlyphScaling {
    Magnitude, ///< Length = scale * |v| (default).
    Constant   ///< Length = scale for every glyph; colour shows the magnitude.
};

/** @brief Where the glyph sits relative to its sample position. */
enum class GlyphAnchor {
    Tail,  ///< The arrow starts at the position (default).
    Center ///< The arrow is centred on the position.
};

/**
 * @class Glyphs
 * @brief Scene node drawing an arrow (or cone, or line) for each sample of a VectorField.
 *
 * Samples are a regular grid over the field's bounds (setSampleGrid()) or positions of your
 * own (setPositions()). All glyphs form one mesh, so they cast shadows and are picked by
 * their triangles (glyphIndex() tells which glyph a RayHit triangle belongs to). With a
 * colormap, glyphs are coloured by vector length; otherwise by the node's material.
 *
 * @code
 * auto field = ivfvis::VectorField::fromGrid(grid, "velocity");
 * auto arrows = ivfvis::Glyphs::create(field);
 * arrows->setSampleGrid({12, 12, 12});
 * arrows->setScale(0.1f);
 * arrows->setColormap(cmap);
 * scene->add(arrows);
 * @endcode
 */
class Glyphs : public ivf::MeshNode {
public:
    explicit Glyphs(VectorFieldPtr field);

    [[nodiscard]] static std::shared_ptr<Glyphs> create(VectorFieldPtr field);

    void setField(VectorFieldPtr field);
    [[nodiscard]] VectorFieldPtr field() const;

    /** @brief Sample on a regular grid of @p counts points over the field's bounds (default 10^3). */
    void setSampleGrid(const glm::ivec3 &counts);

    /** @brief Sample at these positions instead of a grid. */
    void setPositions(std::vector<glm::vec3> positions);

    /** @brief The sample positions in use. */
    [[nodiscard]] const std::vector<glm::vec3> &positions() const;

    void setShape(GlyphShape shape);
    [[nodiscard]] GlyphShape shape() const;

    void setScaling(GlyphScaling scaling);
    [[nodiscard]] GlyphScaling scaling() const;

    /** @brief Length factor (Magnitude) or length (Constant); default 1. */
    void setScale(float scale);
    [[nodiscard]] float scale() const;

    void setAnchor(GlyphAnchor anchor);
    [[nodiscard]] GlyphAnchor anchor() const;

    /** @brief Thickness relative to the length (default 1: shaft radius 5 % of the length). */
    void setThickness(float thickness);
    [[nodiscard]] float thickness() const;

    /** @brief Leave out vectors shorter than this (default 0: only zero vectors). */
    void setMinMagnitude(float magnitude);

    /** @brief Colour by vector length through @p colormap (nullptr: the material's colour). */
    void setColormap(ColormapPtr colormap);
    [[nodiscard]] ColormapPtr colormap() const;

    /** @brief Smallest and largest length of the sampled vectors (invalid if none). */
    [[nodiscard]] ValueRange magnitudeRange() const;

    /** @brief Number of glyphs drawn. */
    [[nodiscard]] std::size_t glyphCount() const;

    /** @brief The glyph a triangle of the mesh belongs to (RayHit::triangle), or -1. */
    [[nodiscard]] int glyphIndex(int triangle) const;

    /** @brief Position of glyph @p index (its sample position). */
    [[nodiscard]] glm::vec3 glyphPosition(std::size_t index) const;

    /** @brief Vector of glyph @p index. */
    [[nodiscard]] glm::vec3 glyphVector(std::size_t index) const;

    /** @brief Sample the field again and rebuild (after the field's data changed). */
    void update();

    void collect(ivf::RenderQueue &queue, const ivf::CollectState &state) override;

private:
    void rebuild();
    void recolor();

    VectorFieldPtr m_field;
    glm::ivec3 m_sampleGrid{10, 10, 10};
    bool m_ownPositions{false};
    std::vector<glm::vec3> m_positions;
    GlyphShape m_shape{GlyphShape::Arrow};
    GlyphScaling m_scaling{GlyphScaling::Magnitude};
    float m_scale{1.0f};
    GlyphAnchor m_anchor{GlyphAnchor::Tail};
    float m_thickness{1.0f};
    float m_minMagnitude{0.0f};
    ColormapPtr m_colormap;

    // Glyphs drawn: sample index, vector; per vertex: magnitude
    std::vector<std::size_t> m_glyphSample;
    std::vector<glm::vec3> m_glyphVector;
    std::vector<float> m_vertexMagnitude;
    int m_trianglesPerGlyph{0};
    const Colormap *m_coloredWith{nullptr};
    std::uint64_t m_coloredVersion{0};
};

using GlyphsPtr = std::shared_ptr<Glyphs>;

} // namespace ivfvis
