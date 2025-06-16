#include <ivf/vignette_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::VignetteEffect::VignetteEffect() : m_vignetteSize(1.5f), m_vignetteSmoothness(1.0f)
{}

std::shared_ptr<VignetteEffect> ivf::VignetteEffect::create()
{
    return std::make_shared<VignetteEffect>();
}

void ivf::VignetteEffect::setSize(float size)
{
    m_vignetteSize = size;
}

float ivf::VignetteEffect::size()
{
    return m_vignetteSize;
}

void ivf::VignetteEffect::setSmoothness(float smoothness)
{
    m_vignetteSmoothness = smoothness;
}

float ivf::VignetteEffect::smoothness()
{
    return m_vignetteSmoothness;
}

void ivf::VignetteEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::vignette_frag_shader_source, "vignette", false);
}

void ivf::VignetteEffect::doUpdateParams()
{
    m_program->uniformFloat("vignetteSize", m_vignetteSize);
    m_program->uniformFloat("vignetteSmoothness", m_vignetteSmoothness);
}

void ivf::VignetteEffect::setupProperties()
{
    addProperty("Size", &m_vignetteSize, "Vignette");
    addProperty("Smoothness", &m_vignetteSmoothness, "Vignette");
}
