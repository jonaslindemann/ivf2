#pragma once

#include <ivf/node.h>

#include <glm/glm.hpp>

namespace ivf {

class TransformNode : public Node {
private:
    glm::vec3 m_pos;
    glm::vec3 m_rotAxis;
    float m_rotAngle;
    glm::vec3 m_scale;
    bool m_useTransform;
    glm::vec3 m_vecRot;
    glm::vec3 m_eulerAngles;

public:
    TransformNode();

    void setPos(glm::vec3 pos);
    glm::vec3 pos();

    void setUseTransform(bool flag);
    bool useTransform();

    void setRotAxis(glm::vec3 axis);
    glm::vec3 rotAxis() const;

    void setRotAngle(float angle);
    float rotAngle() const;

    void setEulerAngles(float ax, float ay, float az);
    void setEulerAngles(glm::vec3 angles);

    void rotateTowards(glm::vec3 target);
    void alignWithAxisAngle(glm::vec3 axis, float angle);
    void rotateToVector(glm::vec3 v);

protected:
    virtual void doPreDraw();
    virtual void doPostDraw();
};

typedef std::shared_ptr<TransformNode> TransformNodePtr;

}; // namespace ivf
