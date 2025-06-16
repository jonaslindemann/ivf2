#include <ivf/chromatic_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::ChromaticEffect::ChromaticEffect() : m_offset(0.01)
{}

ivf::ChromaticEffect::~ChromaticEffect()
{}

std::shared_ptr<ChromaticEffect> ivf::ChromaticEffect::create()
{
    return std::make_shared<ChromaticEffect>();
}

void ivf::ChromaticEffect::setOffset(float offset)
{
    m_offset = offset;
}

float ivf::ChromaticEffect::offset() const
{
    return m_offset;
}

void ivf::ChromaticEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::chromatic_frag_shader_source, "chromatic", false);
}

void ivf::ChromaticEffect::doUpdateParams()
{
    m_program->uniformFloat("offset", m_offset);
}

void ivf::ChromaticEffect::setupProperties()
{
    addProperty("offset", &m_offset, "Chromatic");
}
