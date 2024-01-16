#include <ivf/transform_node.h>

#include <ivf/transform_manager.h>
#include <ivf/utils.h>

using namespace ivf;

TransformNode::TransformNode() : m_pos(0.0f), m_rotAxis(0.0f), m_rotAngle(0.0f), m_scale(1.0f), m_useTransform(true)
{
    m_rotAxis.y = 1.0f;
}

void TransformNode::setPos(glm::vec3 pos)
{
    m_pos = pos;
}

glm::vec3 TransformNode::pos()
{
    return m_pos;
}

void ivf::TransformNode::setUseTransform(bool flag)
{
    m_useTransform = flag;
}

bool ivf::TransformNode::useTransform()
{
    return m_useTransform;
}

void ivf::TransformNode::setRotAxis(glm::vec3 axis)
{
    m_rotAxis = axis;
}

glm::vec3 ivf::TransformNode::rotAxis() const
{
    return m_rotAxis;
}

void ivf::TransformNode::setRotAngle(float angle)
{
    m_rotAngle = angle;
}

float ivf::TransformNode::rotAngle() const
{
    return m_rotAngle;
}

void ivf::TransformNode::rotateTowards(glm::vec3 target)
{
    // FIXME: This is not correct
    auto current = glm::vec3(1.0, 0.0, 0.0);
    auto targetDir = glm::normalize(target);
    auto rotMat = createRotationMatrixTowards(current, targetDir);
    m_rotAxis = glm::vec3(rotMat * glm::vec4(current, 0.0f));
}

void TransformNode::doPreDraw()
{
    Node::doPreDraw();

    if (m_useTransform) {
        xfmMgr()->enableModelMatrix();
        xfmMgr()->pushMatrix();
        xfmMgr()->translate(m_pos);
        if (m_rotAngle != 0.0)
            xfmMgr()->rotate(m_rotAxis.x, m_rotAxis.y, m_rotAxis.z, m_rotAngle);
        if (!(m_scale.x == 1.0f) && (m_scale.y == 1.0f) && (m_scale.z == 1.0f))
            xfmMgr()->scale(m_scale.x, m_scale.y, m_scale.z);
    }
}

void TransformNode::doPostDraw()
{
    if (m_useTransform)
        xfmMgr()->popMatrix();
}
