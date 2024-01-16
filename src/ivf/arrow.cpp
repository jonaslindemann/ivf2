#include <ivf/arrow.h>

using namespace ivf;

Arrow::Arrow()
    : CompositeNode(), m_arrowType(ArrowType::DoubleSided), m_length(1.0f), m_bodyRadius(0.1f), m_coneRadius(0.2f),
      m_arrowBody(nullptr), m_leftArrowCone(nullptr), m_rightArrowCone(nullptr), m_coneLength(0.4f)
{
    this->setUseMaterial(true);

    m_arrowBody = CappedCylinder::create();
    m_leftArrowCone = CappedCone::create();
    m_rightArrowCone = CappedCone::create();

    this->add(m_arrowBody);
    this->add(m_leftArrowCone);
    this->add(m_rightArrowCone);

    this->refresh();
}

std::shared_ptr<Arrow> Arrow::create()
{
    return std::make_shared<Arrow>();
}

void ivf::Arrow::setArrowType(ArrowType arrowType)
{
    m_arrowType = arrowType;
}

ArrowType ivf::Arrow::arrowType() const
{
    return ArrowType();
}

void ivf::Arrow::setLength(GLfloat length)
{
    m_length = length;
}

void ivf::Arrow::setBodyRadius(GLfloat radius)
{
    m_bodyRadius = radius;
}

void ivf::Arrow::setConeRadius(GLfloat radius)
{
    m_coneRadius = radius;
}

void ivf::Arrow::setConeLength(GLfloat length)
{
    m_coneLength = length;
}

GLfloat ivf::Arrow::length() const
{
    return m_length;
}

GLfloat ivf::Arrow::bodyRadius() const
{
    return m_bodyRadius;
}

GLfloat ivf::Arrow::coneRadius() const
{
    return m_coneRadius;
}

GLfloat ivf::Arrow::coneLength() const
{
    return m_coneLength;
}

void ivf::Arrow::refresh()
{
    this->doSetup();
}

void ivf::Arrow::doSetup()
{
    m_arrowBody->set(m_bodyRadius, m_length, 32, 8, 4, 0.0f, 2.0f * glm::pi<float>());
    m_leftArrowCone->set(m_coneRadius, m_coneLength, 32, 8, 4, 0.0f, 2.0f * glm::pi<float>());
    m_rightArrowCone->set(m_coneRadius, m_coneLength, 32, 8, 4, 0.0f, 2.0f * glm::pi<float>());

    m_arrowBody->setUseTransform(true);
    m_leftArrowCone->setUseTransform(true);
    m_rightArrowCone->setUseTransform(true);

    m_arrowBody->setPos(glm::vec3(0.0f, 0.0f, 0.0f));
    m_leftArrowCone->setPos(glm::vec3(0.0f, -m_length / 2.0 - m_coneLength / 2.0, 0.0f));
    m_rightArrowCone->setPos(glm::vec3(0.0f, m_length / 2.0 + m_coneLength / 2.0, 0.0f));

    m_leftArrowCone->setRotAxis(glm::vec3(0.0f, 0.0f, 1.0f));
    m_leftArrowCone->setRotAngle(glm::pi<float>());

    switch (m_arrowType) {
    case ArrowType::DoubleSided:
        m_leftArrowCone->setVisible(true);
        m_rightArrowCone->setVisible(true);
        break;
    case ArrowType::LeftSided:
        m_leftArrowCone->setVisible(true);
        m_rightArrowCone->setVisible(false);
        break;
    case ArrowType::RightSided:
        m_leftArrowCone->setVisible(false);
        m_rightArrowCone->setVisible(true);
        break;
    default:
        break;
    }
}
