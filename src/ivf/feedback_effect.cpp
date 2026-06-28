#include <ivf/feedback_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::FeedbackEffect::FeedbackEffect()
{
    this->setName("Feedback");
}

std::shared_ptr<FeedbackEffect> ivf::FeedbackEffect::create()
{
    return std::make_shared<FeedbackEffect>();
}

void ivf::FeedbackEffect::setFeedbackAmount(float amount)
{
    m_feedbackAmount = amount;
}

float ivf::FeedbackEffect::feedbackAmount() const
{
    return m_feedbackAmount;
}

void ivf::FeedbackEffect::setZoom(float zoom)
{
    m_zoom = zoom;
}

float ivf::FeedbackEffect::zoom() const
{
    return m_zoom;
}

void ivf::FeedbackEffect::setRotation(float radians)
{
    m_rotation = radians;
}

float ivf::FeedbackEffect::rotation() const
{
    return m_rotation;
}

void ivf::FeedbackEffect::setOffset(float x, float y)
{
    m_offsetX = x;
    m_offsetY = y;
}

float ivf::FeedbackEffect::offsetX() const
{
    return m_offsetX;
}

float ivf::FeedbackEffect::offsetY() const
{
    return m_offsetY;
}

void ivf::FeedbackEffect::setColorDecay(glm::vec3 decay)
{
    m_colorDecay = decay;
}

glm::vec3 ivf::FeedbackEffect::colorDecay() const
{
    return m_colorDecay;
}

void ivf::FeedbackEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::feedback_frag_shader_source, "feedback", false);
}

void ivf::FeedbackEffect::doUpdateParams()
{
    m_program->uniformFloat("feedbackAmount", m_feedbackAmount);
    m_program->uniformFloat("zoom", m_zoom);
    m_program->uniformFloat("rotation", m_rotation);
    m_program->uniformFloat("offsetX", m_offsetX);
    m_program->uniformFloat("offsetY", m_offsetY);
    m_program->uniformVec3("colorDecay", m_colorDecay);
}

void ivf::FeedbackEffect::setupProperties()
{
    addProperty("feedbackAmount", &m_feedbackAmount, 0.0f, 1.0f, "Feedback");
    addProperty("zoom", &m_zoom, 0.9f, 1.1f, "Feedback");
    addProperty("rotation", &m_rotation, -0.1f, 0.1f, "Feedback");
    addProperty("offsetX", &m_offsetX, -0.05f, 0.05f, "Feedback");
    addProperty("offsetY", &m_offsetY, -0.05f, 0.05f, "Feedback");
    addPropertyWithRange("colorDecay", &m_colorDecay, 0.0, 1.0, "Feedback");
}
