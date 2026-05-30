#include <ivf/pbr_mesh_node.h>

using namespace ivf;

PBRMeshNode::PBRMeshNode()
{
    m_pbrMat = PBRMaterial::create();
    setMaterial(m_pbrMat);
}

std::shared_ptr<PBRMeshNode> PBRMeshNode::create()
{
    return std::make_shared<PBRMeshNode>();
}

void PBRMeshNode::setPBRMaterial(PBRMaterialPtr mat)
{
    m_pbrMat = mat;
    setMaterial(mat);
}

PBRMaterialPtr PBRMeshNode::pbrMaterial()
{
    return m_pbrMat;
}
