#include <ivf/transformnode.h>

#include <ivf/transformmanager.h>
#include <ivf/utils.h>

using namespace ivf;

TransformNode::TransformNode()
	:m_pos(0.0f),
	 m_rotAxis(0.0f),
	 m_rotAngle(0.0f),
	 m_scale(1.0f)
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

void TransformNode::doPreDraw()
{
	Node::doPreDraw();

	xfmMgr()->enableModelMatrix();
	xfmMgr()->pushMatrix();
	xfmMgr()->translate(m_pos);
	if (m_rotAngle != 0.0)
		xfmMgr()->rotate(m_rotAxis.x, m_rotAxis.y, m_rotAxis.z, m_rotAngle);
	if (!(m_scale.x == 1.0f)&&(m_scale.y == 1.0f)&&(m_scale.z == 1.0f))
		xfmMgr()->scale(m_scale.x, m_scale.y, m_scale.z);
}

void TransformNode::doPostDraw()
{
	xfmMgr()->popMatrix();
}
