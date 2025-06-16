#pragma once

#include <ivf/deformer.h>
#include <ivf/mesh_node.h>
#include <ivf/deformer_stack.h>

namespace ivf {

/**
 * @class DeformableMeshNode
 * @brief Mesh node supporting vertex deformation via a stack of deformers.
 *
 * The DeformableMeshNode class extends MeshNode to allow dynamic deformation of mesh geometry
 * using a stack of Deformer objects. It manages the original vertex data and applies deformers
 * in sequence, supporting both manual and automatic updates. This is useful for animation,
 * morphing, and procedural geometry effects.
 */
class DeformableMeshNode : public MeshNode {
private:
    std::shared_ptr<DeformerStack> m_deformerStack; ///< Stack of deformers to apply to the mesh.
    std::shared_ptr<Vertices> m_originalVertices;   ///< Original (undeformed) vertex data.
    bool m_autoUpdate;                              ///< If true, deformers are applied automatically.

public:
    /**
     * @brief Default constructor.
     */
    DeformableMeshNode();

    /**
     * @brief Factory method to create a shared pointer to a DeformableMeshNode instance.
     * @return std::shared_ptr<DeformableMeshNode> New DeformableMeshNode instance.
     */
    static std::shared_ptr<DeformableMeshNode> create();

    /**
     * @brief Add a deformer to the stack.
     * @param deformer Shared pointer to the deformer to add.
     */
    void addDeformer(std::shared_ptr<Deformer> deformer);

    /**
     * @brief Remove a deformer from the stack.
     * @param deformer Shared pointer to the deformer to remove.
     */
    void removeDeformer(std::shared_ptr<Deformer> deformer);

    /**
     * @brief Remove all deformers from the stack.
     */
    void clearDeformers();

    /**
     * @brief Get the deformer stack.
     * @return std::shared_ptr<DeformerStack> The current deformer stack.
     */
    std::shared_ptr<DeformerStack> deformerStack();

    /**
     * @brief Enable or disable automatic deformation updates.
     * @param autoUpdate True to enable automatic updates, false for manual.
     */
    void setAutoUpdate(bool autoUpdate);

    /**
     * @brief Check if automatic deformation updates are enabled.
     * @return bool True if auto-update is enabled.
     */
    bool autoUpdate() const;

    /**
     * @brief Apply all deformers in the stack to the mesh.
     */
    void applyDeformers();

    /**
     * @brief Reset the mesh to its original (undeformed) state.
     */
    void resetDeformers();

    /**
     * @brief Add a mesh to the node and capture its original vertices.
     * @param mesh Shared pointer to the mesh to add.
     */
    void addMesh(std::shared_ptr<Mesh> mesh);

protected:
    /**
     * @brief Store the original (undeformed) vertex data.
     */
    void storeOriginalVertices();

    /**
     * @brief Draw the mesh, applying deformers if auto-update is enabled.
     */
    virtual void doDraw() override;
};

}; // namespace ivf
