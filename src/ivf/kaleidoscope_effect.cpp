#include <ivf/kaleidoscope_effect.h>
#include <ivf/post_shaders.h>

using namespace ivf;

KaleidoscopeEffect::KaleidoscopeEffect()
{
    setName("Kaleidoscope");
}

std::shared_ptr<KaleidoscopeEffect> KaleidoscopeEffect::create()
{
    return std::make_shared<KaleidoscopeEffect>();
}

void KaleidoscopeEffect::setSegments(float segments) { m_segments = segments; }
void KaleidoscopeEffect::setRotation(float rotation) { m_rotation = rotation; }

float KaleidoscopeEffect::segments() const { return m_segments; }
float KaleidoscopeEffect::rotation() const { return m_rotation; }

void KaleidoscopeEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::kaleidoscope_frag_shader_source, "kaleidoscope", false);
}

void KaleidoscopeEffect::doUpdateParams()
{
    m_program->uniformFloat("segments", m_segments);
    m_program->uniformFloat("rotation", m_rotation);
}

void KaleidoscopeEffect::setupProperties()
{
    addProperty("segments", &m_segments, 2.0f, 16.0f, "Kaleidoscope");
    addProperty("rotation", &m_rotation, -3.14159f, 3.14159f, "Kaleidoscope");
}
