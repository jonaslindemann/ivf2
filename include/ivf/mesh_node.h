#pragma once

#include <ivf/transform_node.h>

#include <generator/generator.hpp>

#include <ivf/mesh.h>
#include <ivf/material.h>

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
    static std::shared_ptr<MeshNode> create();

    /**
     * @brief Add a mesh to the node.
     * @param mesh Shared pointer to the mesh to add.
     */
    void addMesh(std::shared_ptr<Mesh> mesh);

    /**
     * @brief Get the list of meshes managed by this node.
     * @return std::vector<std::shared_ptr<Mesh>> Vector of mesh pointers.
     */
    std::vector<std::shared_ptr<Mesh>> meshes();

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
     * @brief Update vertex data for all meshes.
     */
    void updateVertices();

    /**
     * @brief Update normal data for all meshes.
     */
    void updateNormals();

    /**
     * @brief Print mesh node information for debugging.
     */
    void print();

protected:
    /**
     * @brief Draw the mesh node (called by the scene graph).
     */
    virtual void doDraw();

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
