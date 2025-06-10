#pragma once

#include <ivf/deformer.h>
#include <ivf/mesh_node.h>
#include <ivf/deformer_stack.h>

namespace ivf {

class DeformableMeshNode : public MeshNode {
private:
    std::shared_ptr<DeformerStack> m_deformerStack;
    std::shared_ptr<Vertices> m_originalVertices;
    bool m_autoUpdate;

public:
    DeformableMeshNode();

    static std::shared_ptr<DeformableMeshNode> create();

    void addDeformer(std::shared_ptr<Deformer> deformer);
    void removeDeformer(std::shared_ptr<Deformer> deformer);
    void clearDeformers();

    std::shared_ptr<DeformerStack> deformerStack();

    void setAutoUpdate(bool autoUpdate);
    bool autoUpdate() const;

    void applyDeformers();
    void resetDeformers();

    // Override to capture original vertices
    void addMesh(std::shared_ptr<Mesh> mesh);

protected:
    void storeOriginalVertices();
    virtual void doDraw() override;
};

}; // namespace ivf
