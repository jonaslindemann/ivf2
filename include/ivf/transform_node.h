#pragma once

#include <ivf/node.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * TransformNode class
 *
 * This class is used to define a node that can be transformed in 3D space. It can be translated,
 * rotated, and scaled. The transformation can be applied to the node before or after drawing it.
 */
class TransformNode : public Node {
private:
    glm::vec3 m_pos;
    glm::vec3 m_rotAxis;
    float m_rotAngle;
    glm::vec3 m_scale;
    bool m_useTransform;
    glm::vec3 m_vecRot;
    glm::vec3 m_eulerAngles;
    glm::vec3 m_storedPos;

public:
    TransformNode();

    void setPos(glm::vec3 pos);
    glm::vec3 pos() const;

    void setUseTransform(bool flag);
    bool useTransform() const;

    void setRotAxis(glm::vec3 axis);
    glm::vec3 rotAxis() const;

    void setRotAngle(float angle);
    float rotAngle() const;

    void setEulerAngles(float ax, float ay, float az);
    void setEulerAngles(glm::vec3 angles);

    void rotateTowards(glm::vec3 target);
    void alignWithAxisAngle(glm::vec3 axis, float angle);
    void rotateToVector(glm::vec3 v);

    void storePos();
    void restorePos();

    glm::vec3 storedPos() const;

    void setScale(glm::vec3 scale);
    glm::vec3 scale() const;

    glm::mat4 localTransform() const;
    glm::mat4 globalTransform() const;

    glm::vec3 worldPos() const;

protected:
    virtual void doPreDraw();
    virtual void doPostDraw();
};

typedef std::shared_ptr<TransformNode> TransformNodePtr;

}; // namespace ivf
