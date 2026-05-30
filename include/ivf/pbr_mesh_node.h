#pragma once

#include <ivf/mesh_node.h>
#include <ivf/pbr_material.h>

namespace ivf {

/**
 * @class PBRMeshNode
 * @brief MeshNode with a built-in PBRMaterial for physically-based rendering.
 *
 * Convenience wrapper that keeps a typed PBRMaterialPtr alongside the node.
 * Because PBRMaterial inherits from Material, the PBR apply()/unapply() calls
 * are handled automatically by the Node draw chain — no draw-method overrides
 * are required here.
 *
 * Usage:
 * @code
 * auto node = PBRMeshNode::create();
 * node->createFromGenerator(sphereGenerator, mat);
 * node->pbrMaterial()->setRoughness(0.3f);
 * node->pbrMaterial()->setMetallic(0.9f);
 * scene->add(node);
 * @endcode
 */
class PBRMeshNode : public MeshNode {
    PBRMaterialPtr m_pbrMat;

public:
    PBRMeshNode();
    ~PBRMeshNode() override = default;

    static std::shared_ptr<PBRMeshNode> create();

    /**
     * @brief Assign a PBRMaterial and register it as the node's material.
     */
    void setPBRMaterial(PBRMaterialPtr mat);

    /**
     * @brief Get the node's PBRMaterial (created automatically if none was assigned).
     */
    PBRMaterialPtr pbrMaterial();
};

using PBRMeshNodePtr = std::shared_ptr<PBRMeshNode>;

} // namespace ivf
