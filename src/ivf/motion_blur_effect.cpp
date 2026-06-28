#include <ivf/motion_blur_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::MotionBlurEffect::MotionBlurEffect()
{
    this->setName("MotionBlur");
}

std::shared_ptr<MotionBlurEffect> ivf::MotionBlurEffect::create()
{
    return std::make_shared<MotionBlurEffect>();
}

void ivf::MotionBlurEffect::setBlendFactor(float blendFactor)
{
    m_blendFactor = blendFactor;
}

float ivf::MotionBlurEffect::blendFactor() const
{
    return m_blendFactor;
}

void ivf::MotionBlurEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::motion_blur_frag_shader_source, "motion_blur", false);
}

void ivf::MotionBlurEffect::doUpdateParams()
{
    m_program->uniformFloat("blendFactor", m_blendFactor);
}

void ivf::MotionBlurEffect::setupProperties()
{
    addProperty("blendFactor", &m_blendFactor, 0.0f, 0.99f, "Motion Blur");
}
