#include <ivf/transform_node.h>

#include <ivf/transform_manager.h>
#include <ivf/utils.h>

using namespace ivf;

TransformNode::TransformNode()
    : m_pos(0.0f), m_rotAxis(0.0f), m_rotAngle(0.0f), m_scale(1.0f), m_useTransform(true), m_vecRot(0.0f),
      m_eulerAngles(0.0f), m_storedPos(0.0f), m_autoUpdateBoundingBox(true)
{
    m_rotAxis.y = 1.0f;
}

void TransformNode::setPos(glm::vec3 pos)
{
    m_pos = pos;
}

glm::vec3 TransformNode::pos() const
{
    return m_pos;
}

void ivf::TransformNode::setUseTransform(bool flag)
{
    m_useTransform = flag;
}

bool ivf::TransformNode::useTransform() const
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

void ivf::TransformNode::setEulerAngles(float ax, float ay, float az)
{
    m_eulerAngles = glm::vec3(ax, ay, az);
}

void ivf::TransformNode::setEulerAngles(glm::vec3 angles)
{
    m_eulerAngles = angles;
}

void ivf::TransformNode::rotateTowards(glm::vec3 target)
{
    auto current = glm::vec3(1.0, 0.0, 0.0);
    auto targetDir = glm::normalize(target);
    auto rotMat = createRotationMatrixTowards(current, targetDir);
    m_rotAxis = glm::vec3(rotMat * glm::vec4(current, 0.0f));
}

void ivf::TransformNode::alignWithAxisAngle(glm::vec3 axis, float angle)
{
    m_rotAxis = axis;
    m_rotAngle = angle;
}

void ivf::TransformNode::rotateToVector(glm::vec3 v)
{
    m_vecRot = v;
}

void ivf::TransformNode::storePos()
{
    m_storedPos = m_pos;
}

void ivf::TransformNode::restorePos()
{
    m_pos = m_storedPos;
}

glm::vec3 ivf::TransformNode::storedPos() const
{
    return m_storedPos;
}

void ivf::TransformNode::setScale(glm::vec3 scale)
{
    m_scale = scale;
}

glm::vec3 ivf::TransformNode::scale() const
{
    return m_scale;
}

glm::mat4 ivf::TransformNode::localTransform() const
{
    // Create matrix for this node's transform
    glm::mat4 localTransform = glm::mat4(1.0f);

    // Apply translation
    localTransform = glm::translate(localTransform, m_pos);

    // Apply rotation and scale similar to how it's done in doPreDraw()
    if ((abs(m_eulerAngles.x) > 0.0) || (abs(m_eulerAngles.y) > 0.0) || (abs(m_eulerAngles.z) > 0.0))
    {
        localTransform = glm::rotate(localTransform, glm::radians(m_eulerAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
        localTransform = glm::rotate(localTransform, glm::radians(m_eulerAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        localTransform = glm::rotate(localTransform, glm::radians(m_eulerAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // Apply other rotations similarly...

    if (m_rotAngle != 0.0)
        localTransform = glm::rotate(localTransform, glm::radians(m_rotAngle), m_rotAxis);

    localTransform = glm::scale(localTransform, m_scale);

    return localTransform;
}

glm::mat4 ivf::TransformNode::globalTransform() const
{
    glm::mat4 globalTransform = localTransform();

    // Traverse up the hierarchy using shared_ptr
    std::shared_ptr<Node> p = parent();
    if (p)
    {
        auto parentTransform = std::dynamic_pointer_cast<TransformNode>(p);
        if (parentTransform)
            globalTransform = parentTransform->globalTransform() * globalTransform;
    }

    return globalTransform;
}

glm::vec3 TransformNode::worldPos() const
{
    glm::mat4 globalTransform = localTransform();

    // Traverse up the hierarchy using shared_ptr
    std::shared_ptr<Node> p = parent();
    while (p)
    {
        auto parentTransform = std::dynamic_pointer_cast<TransformNode>(p);
        if (parentTransform)
            globalTransform = parentTransform->localTransform() * globalTransform;
        p = p->parent();
    }

    // Extract and return the position component
    return glm::vec3(globalTransform[3]);
}

void TransformNode::doPreDraw()
{
    Node::doPreDraw();

    if (m_useTransform)
    {
        xfmMgr()->enableModelMatrix();
        xfmMgr()->pushMatrix();
        xfmMgr()->translate(m_pos);
        if ((abs(m_eulerAngles.x) > 0.0) || (abs(m_eulerAngles.y) > 0.0) || (abs(m_eulerAngles.z) > 0.0))
            xfmMgr()->rotateDeg(m_eulerAngles.x, m_eulerAngles.y, m_eulerAngles.z);
        if ((m_vecRot.x > 0.0) || (m_vecRot.y > 0.0) || (m_vecRot.z > 0.0))
            xfmMgr()->rotateToVector(m_vecRot);
        if (m_rotAngle != 0.0)
            xfmMgr()->rotateDeg(m_rotAxis.x, m_rotAxis.y, m_rotAxis.z, m_rotAngle);
        if (!(m_scale.x == 1.0f) && (m_scale.y == 1.0f) && (m_scale.z == 1.0f))
            xfmMgr()->scale(m_scale.x, m_scale.y, m_scale.z);
    }
}

void TransformNode::doPostDraw()
{
    if (m_useTransform)
        xfmMgr()->popMatrix();
}

void ivf::TransformNode::setupProperties()
{
    Node::setupProperties();
    addProperty("Position", &m_pos, "Transform");
    addProperty("Use Transform", &m_useTransform, "Transform");
    addProperty("Rotation Axis", &m_rotAxis, "Transform");
    addProperty("Rotation Angle", &m_rotAngle, "Transform");
    addProperty("Euler Angles", &m_eulerAngles, "Transform");
    addProperty("Stored Position", &m_storedPos, "Transform");
    addProperty("Scale", &m_scale, "Transform");
    addProperty("Auto Update Bbox", &m_autoUpdateBoundingBox, "Transform");
}

BoundingBox TransformNode::localBoundingBox() const
{
    return m_localBbox;
}

BoundingBox TransformNode::worldBoundingBox() const
{
    if (!m_localBbox.isValid())
        return BoundingBox();
        
    return m_localBbox.transform(globalTransform());
}

void TransformNode::setLocalBoundingBox(const BoundingBox& bbox)
{
    m_localBbox = bbox;
}

bool TransformNode::hasValidBoundingBox() const
{
    return m_localBbox.isValid();
}

void TransformNode::setAutoUpdateBoundingBox(bool autoUpdate)
{
    m_autoUpdateBoundingBox = autoUpdate;
}

bool TransformNode::autoUpdateBoundingBox() const
{
    return m_autoUpdateBoundingBox;
}
