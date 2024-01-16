#pragma once

#include <ivf/arrow.h>
#include <ivf/capped_cone.h>
#include <ivf/capped_cylinder.h>
#include <ivf/composite_node.h>

namespace ivf {

enum class ArrowType {
    DoubleSided,
    LeftSided,
    RightSided
};

class Arrow : public CompositeNode {
private:
    ArrowType m_arrowType;
    CappedCylinderPtr m_arrowBody;
    CappedConePtr m_leftArrowCone;
    CappedConePtr m_rightArrowCone;
    GLfloat m_length;
    GLfloat m_bodyRadius;
    GLfloat m_coneRadius;
    GLfloat m_coneLength;

public:
    Arrow();

    static std::shared_ptr<Arrow> create();

    void setArrowType(ArrowType arrowType);
    ArrowType arrowType() const;

    void setLength(GLfloat length);
    void setBodyRadius(GLfloat radius);
    void setConeRadius(GLfloat radius);
    void setConeLength(GLfloat length);

    GLfloat length() const;
    GLfloat bodyRadius() const;
    GLfloat coneRadius() const;
    GLfloat coneLength() const;

    void refresh();

protected:
    virtual void doSetup();
};

typedef std::shared_ptr<Arrow> ArrowPtr;

} // namespace ivf
