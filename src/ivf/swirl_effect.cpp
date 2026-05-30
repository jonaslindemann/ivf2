#include <ivf/swirl_effect.h>
#include <ivf/post_shaders.h>

using namespace ivf;

SwirlEffect::SwirlEffect()
{
    setName("Swirl");
}

std::shared_ptr<SwirlEffect> SwirlEffect::create()
{
    return std::make_shared<SwirlEffect>();
}

void SwirlEffect::setRadius(float radius) { m_radius = radius; }
void SwirlEffect::setAngle(float angle) { m_angle = angle; }

float SwirlEffect::radius() const { return m_radius; }
float SwirlEffect::angle() const { return m_angle; }

void SwirlEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::swirl_frag_shader_source, "swirl", false);
}

void SwirlEffect::doUpdateParams()
{
    m_program->uniformFloat("radius", m_radius);
    m_program->uniformFloat("angle", m_angle);
}

void SwirlEffect::setupProperties()
{
    addProperty("radius", &m_radius, 0.0f, 1.0f, "Swirl");
    addProperty("angle", &m_angle, -6.28318f, 6.28318f, "Swirl");
}
