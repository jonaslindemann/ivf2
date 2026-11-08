#include <ivf/depth_of_field_effect.h>

#include <ivf/post_shaders.h>

#include <algorithm>

using namespace ivf;

DepthOfFieldEffect::DepthOfFieldEffect()
{
    setName("Depth of Field");
}

std::shared_ptr<DepthOfFieldEffect> DepthOfFieldEffect::create()
{
    return std::make_shared<DepthOfFieldEffect>();
}

void DepthOfFieldEffect::setFocusDistance(float distance)
{
    m_focusDistance = std::max(distance, 0.0f);
}

float DepthOfFieldEffect::focusDistance() const
{
    return m_focusDistance;
}

void DepthOfFieldEffect::setFocusRange(float range)
{
    m_focusRange = std::max(range, 0.0f);
}

float DepthOfFieldEffect::focusRange() const
{
    return m_focusRange;
}

void DepthOfFieldEffect::setBlurRange(float range)
{
    m_blurRange = std::max(range, 0.001f);
}

float DepthOfFieldEffect::blurRange() const
{
    return m_blurRange;
}

void DepthOfFieldEffect::setMaxBlur(float pixels)
{
    m_maxBlur = std::clamp(pixels, 0.0f, 64.0f);
}

float DepthOfFieldEffect::maxBlur() const
{
    return m_maxBlur;
}

void DepthOfFieldEffect::setRadiusStep(float step)
{
    m_radiusStep = std::clamp(step, 0.25f, 4.0f);
}

float DepthOfFieldEffect::radiusStep() const
{
    return m_radiusStep;
}

void DepthOfFieldEffect::setAutoFocus(bool flag)
{
    m_autoFocus = flag;
}

bool DepthOfFieldEffect::autoFocus() const
{
    return m_autoFocus;
}

void DepthOfFieldEffect::setShowBlur(bool flag)
{
    m_showBlur = flag;
}

bool DepthOfFieldEffect::showBlur() const
{
    return m_showBlur;
}

void DepthOfFieldEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, ivf::dof_frag_shader_source,
                                         "depth_of_field", false);
}

void DepthOfFieldEffect::doUpdateParams()
{
    m_program->uniformFloat("dofFocusDistance", m_focusDistance);
    m_program->uniformFloat("dofFocusRange", m_focusRange);
    m_program->uniformFloat("dofBlurRange", m_blurRange);
    m_program->uniformFloat("dofMaxBlur", m_maxBlur);
    m_program->uniformFloat("dofRadiusStep", m_radiusStep);
    m_program->uniformBool("dofAutoFocus", m_autoFocus);
    m_program->uniformBool("dofShowBlur", m_showBlur);
}

void DepthOfFieldEffect::setupProperties()
{
    addProperty("Focus distance", &m_focusDistance, 0.0, 200.0, "Depth of Field");
    addProperty("Focus range", &m_focusRange, 0.0, 50.0, "Depth of Field");
    addProperty("Blur range", &m_blurRange, 0.001, 100.0, "Depth of Field");
    addProperty("Max blur", &m_maxBlur, 0.0, 64.0, "Depth of Field");
    addProperty("Radius step", &m_radiusStep, 0.25, 4.0, "Depth of Field");
    addProperty("Auto focus", &m_autoFocus, "Depth of Field");
    addProperty("Show blur", &m_showBlur, "Depth of Field");
}
