#pragma once

#include <ivf/mesh_node.h>
#include <glm/glm.hpp>
#include <vector>

namespace ivf {

/**
 * @brief Scene node for runtime geometry built from raw vertex arrays.
 *
 * Ideal for generative art, attractors, audio waveforms, and L-systems
 * where geometry changes every frame or is computed procedurally.
 *
 * Builder pattern:
 * @code
 * auto dm = DynamicMesh::create();
 * dm->beginMesh(GL_LINE_STRIP);
 *   for (int i = 0; i < N; ++i) {
 *       dm->vertex({x, y, z});
 *       dm->color({r, g, b, 1});
 *   }
 * dm->endMesh();
 * scene->add(dm);
 *
 * // Every frame: update only positions (no reallocation if count unchanged)
 * dm->beginUpdate();
 *   for (auto& p : newPositions) dm->vertex(p);
 * dm->endUpdate();
 * @endcode
 */
class DynamicMesh : public MeshNode {
private:
    struct RawVertex {
        glm::vec3 pos{0,0,0};
        glm::vec3 normal{0,1,0};
        glm::vec2 uv{0,0};
        glm::vec4 color{1,1,1,1};
    };

    std::vector<RawVertex> m_buffer;
    std::vector<unsigned int> m_indexBuffer;

    RawVertex m_current;              // accumulates per-vertex attributes
    GLenum m_primType{GL_TRIANGLES};
    bool m_building{false};
    bool m_updatingPositions{false};
    int  m_updateIdx{0};

    float m_pointSize{1.0f};
    float m_lineWidth{1.0f};

    void commitMesh();

public:
    DynamicMesh();

    static std::shared_ptr<DynamicMesh> create();

    // ---- Builder API -------------------------------------------------------

    /**
     * Begin accumulating a new mesh. Clears any previous geometry.
     * @param primType GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_LINE_STRIP, GL_LINES, GL_POINTS, etc.
     */
    void beginMesh(GLenum primType = GL_TRIANGLES);

    void vertex(glm::vec3 pos);
    void normal(glm::vec3 n);
    void texCoord(glm::vec2 uv);
    void color(glm::vec4 c);
    void index(unsigned int i);

    /** Upload geometry to the GPU. The mesh is now visible. */
    void endMesh();

    // ---- Streaming update (positions only, no reallocation) ----------------

    /**
     * Begin updating only vertex positions on an existing mesh.
     * Vertex count must match the count from the last endMesh() call.
     */
    void beginUpdate();

    /** Update the next vertex position in sequence. */
    void vertex(glm::vec3 pos, bool updateMode);

    /** Upload updated positions to the GPU. */
    void endUpdate();

    // ---- Convenience: replace all positions at once ------------------------

    /**
     * Replace all vertex positions in one call.
     * @param positions Must have the same number of entries as the current mesh.
     */
    void updatePositions(const std::vector<glm::vec3>& positions);

    // ---- Appearance --------------------------------------------------------

    void setPointSize(float size);
    void setLineWidth(float width);

    [[nodiscard]] float pointSize() const { return m_pointSize; }
    [[nodiscard]] float lineWidth() const { return m_lineWidth; }

    /** Number of vertices in the current built mesh. */
    [[nodiscard]] int vertexCount() const;
};

using DynamicMeshPtr = std::shared_ptr<DynamicMesh>;

} // namespace ivf
