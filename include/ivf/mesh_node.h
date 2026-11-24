#pragma once

#include <ivf/transform_node.h>

#include <generator/generator.hpp>

#include <ivf/mesh.h>
#include <ivf/material.h>
#include <ivf/extrusion_builder.h>

#include <vector>

namespace ivf {

/**
 * @class MeshNode
 * @brief Scene node that manages and renders a collection of Mesh objects.
 *
 * The MeshNode class provides a node in the scene graph that can contain and manage
 * multiple Mesh objects. It supports adding, creating, and accessing meshes, as well as
 * updating, refreshing, and printing mesh data. Inherits from TransformNode, allowing
 * transformation and hierarchical placement in the scene.
 */
class MeshNode : public TransformNode {
private:
    std::shared_ptr<Mesh> m_normalVisMesh;
    bool m_showNormals{false};
    float m_normalLength{1.0f};

    Mesh *m_drawMesh{nullptr};               ///< Mesh of the item being drawn (doDrawPass()); nullptr: all.
    RenderContext *m_drawContext{nullptr};   ///< Context of the item being drawn (doDrawPass()).
    std::span<const glm::mat4> m_drawInstances; ///< World matrices of a batch (doDrawPass()); empty: one.

protected:
    std::vector<std::shared_ptr<Mesh>> m_meshes; ///< List of meshes managed by this node.

public:
    /**
     * @brief Default constructor.
     */
    MeshNode();

    /**
     * @brief Factory method to create a shared pointer to a MeshNode instance.
     * @return std::shared_ptr<MeshNode> New MeshNode instance.
     */
    [[nodiscard]] static std::shared_ptr<MeshNode> create();

    /**
     * @brief Add a mesh to the node.
     * @param mesh Shared pointer to the mesh to add.
     */
    void addMesh(std::shared_ptr<Mesh> mesh);

    /**
     * @brief Get the list of meshes managed by this node.
     * @return std::vector<std::shared_ptr<Mesh>> Vector of mesh pointers.
     */
    std::vector<std::shared_ptr<Mesh>> meshes() const;

    /**
     * @brief Create and add a new mesh to the node.
     * @param vsize Number of vertices.
     * @param isize Number of indices (default 0).
     * @param primType OpenGL primitive type (default GL_TRIANGLES).
     * @param usage OpenGL buffer usage (default GL_STATIC_DRAW).
     */
    void newMesh(int vsize, int isize = 0, GLuint primType = GL_TRIANGLES, GLenum usage = GL_STATIC_DRAW);

    /**
     * @brief Get the mesh at the specified index.
     * @param idx Index of the mesh (default 0).
     * @return std::shared_ptr<Mesh> Mesh pointer.
     */
    std::shared_ptr<Mesh> mesh(int idx = 0);

    /**
     * @brief Get the last mesh added to the node.
     * @return std::shared_ptr<Mesh> Mesh pointer.
     */
    std::shared_ptr<Mesh> lastMesh();

    /**
     * @brief Get the current mesh (last mesh added).
     * @return std::shared_ptr<Mesh> Mesh pointer.
     */
    std::shared_ptr<Mesh> currentMesh();

    /**
     * @brief Remove all meshes from the node.
     */
    void clear();

    /**
     * @brief Enable or disable wireframe rendering for all meshes.
     * @param flag True for wireframe, false for filled.
     */
    void setWireframe(bool flag);

    /**
     * @brief Create mesh data from a generator (vertices and triangles).
     * @param vertices Generator for mesh vertices.
     * @param triangles Generator for mesh triangles.
     */
    void createFromGenerator(generator::AnyGenerator<generator::MeshVertex> &vertices,
                             generator::AnyGenerator<generator::Triangle> &triangles);

    /**
     * @brief Create an indexed triangle mesh from CPU-side mesh data.
     *
     * Replaces the node's current meshes with a single mesh built from @p data (as produced by
     * ExtrusionBuilder). Vertex normals from @p data are used directly. The local bounding box is
     * updated afterwards.
     * @param data Indexed mesh data (positions, normals, texCoords, colors, indices).
     */
    void createFromMeshData(const MeshData &data);

    /**
     * @brief Create debug mesh data from a generator (vertices and triangles).
     * @param vertices Generator for mesh vertices.
     * @param triangles Generator for mesh triangles.
     */
    void debugFromGenerator(generator::AnyGenerator<generator::MeshVertex> &vertices,
                            generator::AnyGenerator<generator::Triangle> &triangles);

    /**
     * @brief Refresh all mesh data (update OpenGL buffers).
     */
    void refresh();

    /**
     * @brief Mark the vertex data of all meshes as changed and recompute the bounding box.
     *
     * Call it after changing vertices; the bounding box is used for culling. If you change
     * a Mesh's vertices directly (Mesh::updateVertices()), call updateBoundingBox() too.
     */
    void updateVertices();

    /**
     * @brief Update normal data for all meshes.
     */
    void updateNormals();

    /**
     * @brief Automatically compute and update the local bounding box from mesh data.
     */
    void updateBoundingBox();

    /**
     * @brief Print mesh node information for debugging.
     */
    void print();

    void setShowNormals(bool show, float length = 1.0f);
    void updateNormalVisualization();
    bool showNormals() const;

    /**
     * @brief Add one draw item per enabled mesh (DrawItem::mesh).
     *
     * Each mesh is then culled, sorted and, if its material is transparent, blended on its
     * own; a mesh's material (Mesh::setMaterial()) decides its layer and sort order. All
     * items share the node's world matrix and bounding box, and the node's hooks
     * (doPreDraw(), doDraw(), doPostDraw()) run once per item. A node without meshes adds
     * a single item like any other node.
     */
    void collect(RenderQueue &queue, const CollectState &state) override;

protected:
    /**
     * @brief Draw the meshes (called by the scene graph).
     *
     * Draws the mesh of the item being drawn (see collect()), or all enabled meshes when
     * called outside the renderer. For a batch of identical items (DrawContext::instances)
     * it draws all copies with Mesh::drawInstances(). Subclasses that override it should
     * call it to draw.
     */
    virtual void doDraw();

    /**
     * @brief Draw the item's mesh with the renderer's context, then call doDraw().
     */
    void doDrawPass(const DrawContext &context) override;

    /**
     * @brief Internal setup method for initializing the mesh node.
     */
    virtual void doSetup();
};

/**
 * @typedef MeshNodePtr
 * @brief Shared pointer type for MeshNode.
 */
typedef std::shared_ptr<MeshNode> MeshNodePtr;

}; // namespace ivf
