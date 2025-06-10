#include <ivf/deformable_mesh_node.h>
#include <ivf/utils.h>
#include <ivf/deformer_stack.h>

using namespace ivf;

DeformableMeshNode::DeformableMeshNode() : MeshNode(), m_deformerStack(DeformerStack::create()), m_autoUpdate(false)
{}

std::shared_ptr<DeformableMeshNode> DeformableMeshNode::create()
{
    return std::make_shared<DeformableMeshNode>();
}

void DeformableMeshNode::addDeformer(std::shared_ptr<Deformer> deformer)
{
    m_deformerStack->addDeformer(deformer);
}

void DeformableMeshNode::removeDeformer(std::shared_ptr<Deformer> deformer)
{
    m_deformerStack->removeDeformer(deformer);
}

void DeformableMeshNode::clearDeformers()
{
    m_deformerStack->clear();
}

std::shared_ptr<DeformerStack> DeformableMeshNode::deformerStack()
{
    return m_deformerStack;
}

void DeformableMeshNode::setAutoUpdate(bool autoUpdate)
{
    m_autoUpdate = autoUpdate;
}

bool DeformableMeshNode::autoUpdate() const
{
    return m_autoUpdate;
}

void DeformableMeshNode::applyDeformers()
{
    if (!m_meshes.empty() && m_originalVertices)
    {
        m_deformerStack->setInput(m_originalVertices);
        m_deformerStack->apply();

        auto deformedVertices = m_deformerStack->getOutput();
        if (deformedVertices)
        {
            // Update mesh vertices
            auto mesh = m_meshes[0]; // Apply to first mesh for simplicity
            auto meshVertices = mesh->vertices();
            std::memcpy(meshVertices->data(), deformedVertices->data(), deformedVertices->memSize());
            mesh->updateVertices();
            mesh->updateNormals();
        }
    }
}

void DeformableMeshNode::resetDeformers()
{
    m_deformerStack->reset();
    if (!m_meshes.empty() && m_originalVertices)
    {
        auto mesh = m_meshes[0];
        auto meshVertices = mesh->vertices();
        std::memcpy(meshVertices->data(), m_originalVertices->data(), m_originalVertices->memSize());
        mesh->updateVertices();
        mesh->updateNormals();
    }
}

void DeformableMeshNode::addMesh(std::shared_ptr<Mesh> mesh)
{
    MeshNode::addMesh(mesh);
    storeOriginalVertices();
}

void DeformableMeshNode::storeOriginalVertices()
{
    if (!m_meshes.empty())
    {
        auto meshVertices = m_meshes[0]->vertices();
        m_originalVertices = std::make_shared<Vertices>(meshVertices->rows());
        std::memcpy(m_originalVertices->data(), meshVertices->data(), meshVertices->memSize());
    }
}

void DeformableMeshNode::doDraw()
{
    if (m_autoUpdate)
    {
        applyDeformers();
    }
    MeshNode::doDraw();
}
