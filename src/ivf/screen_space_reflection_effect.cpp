#include <ivf/screen_space_reflection_effect.h>

#include <ivf/post_shaders.h>
#include <ivf/screen_space_shaders.h>

#include <algorithm>
#include <cmath>

using namespace ivf;

ScreenSpaceReflectionEffect::ScreenSpaceReflectionEffect()
{
    setName("Screen-space reflections");
}

std::shared_ptr<ScreenSpaceReflectionEffect> ScreenSpaceReflectionEffect::create()
{
    return std::make_shared<ScreenSpaceReflectionEffect>();
}

void ScreenSpaceReflectionEffect::setStrength(float strength)
{
    m_strength = std::clamp(strength, 0.0f, 1.0f);
}

float ScreenSpaceReflectionEffect::strength() const
{
    return m_strength;
}

void ScreenSpaceReflectionEffect::setMaxDistance(float distance)
{
    m_maxDistance = std::max(distance, 0.01f);
}

float ScreenSpaceReflectionEffect::maxDistance() const
{
    return m_maxDistance;
}

void ScreenSpaceReflectionEffect::setSteps(int steps)
{
    m_steps = std::clamp(steps, 4, 128);
}

int ScreenSpaceReflectionEffect::steps() const
{
    return m_steps;
}

void ScreenSpaceReflectionEffect::setThickness(float thickness)
{
    m_thickness = std::max(thickness, 0.0f);
}

float ScreenSpaceReflectionEffect::thickness() const
{
    return m_thickness;
}

void ScreenSpaceReflectionEffect::setRoughness(float roughness)
{
    m_roughness = std::clamp(roughness, 0.0f, 1.0f);
}

float ScreenSpaceReflectionEffect::roughness() const
{
    return m_roughness;
}

void ScreenSpaceReflectionEffect::setFresnel(float fresnel)
{
    m_fresnel = std::clamp(fresnel, 0.0f, 1.0f);
}

float ScreenSpaceReflectionEffect::fresnel() const
{
    return m_fresnel;
}

void ScreenSpaceReflectionEffect::setEdgeFade(float fraction)
{
    m_edgeFade = std::clamp(fraction, 0.0f, 0.5f);
}

float ScreenSpaceReflectionEffect::edgeFade() const
{
    return m_edgeFade;
}

void ScreenSpaceReflectionEffect::setSurfaceAngle(float degrees)
{
    m_surfaceAngle = std::clamp(degrees, 0.0f, 180.0f);
}

float ScreenSpaceReflectionEffect::surfaceAngle() const
{
    return m_surfaceAngle;
}

void ScreenSpaceReflectionEffect::setUpDirection(const glm::vec3 &direction)
{
    if (glm::length(direction) > 0.0f)
        m_upDirection = glm::normalize(direction);
}

glm::vec3 ScreenSpaceReflectionEffect::upDirection() const
{
    return m_upDirection;
}

void ScreenSpaceReflectionEffect::setShowReflections(bool flag)
{
    m_showReflections = flag;
}

bool ScreenSpaceReflectionEffect::showReflections() const
{
    return m_showReflections;
}

void ScreenSpaceReflectionEffect::doLoad()
{
    m_program = compileProgram(ivf::render_to_texture_vert_shader_source_330, ivf::ssr_frag_shader_source,
                               "screen_space_reflection");
}

void ScreenSpaceReflectionEffect::doUpdateParams()
{
    m_program->uniformFloat("ssrStrength", m_strength);
    m_program->uniformFloat("ssrMaxDistance", m_maxDistance);
    m_program->uniformInt("ssrSteps", m_steps);
    m_program->uniformFloat("ssrThickness", m_thickness);
    m_program->uniformFloat("ssrRoughness", m_roughness);
    m_program->uniformFloat("ssrFresnel", m_fresnel);
    m_program->uniformFloat("ssrEdgeFade", m_edgeFade);
    m_program->uniformFloat("ssrSurfaceCos", std::cos(glm::radians(std::clamp(m_surfaceAngle, 0.0f, 180.0f))));
    m_program->uniformVec3("ssrUp", m_upDirection);
    m_program->uniformBool("ssrShow", m_showReflections);
}

void ScreenSpaceReflectionEffect::setupProperties()
{
    addProperty("Strength", &m_strength, 0.0, 1.0, "Reflections");
    addProperty("Max distance", &m_maxDistance, 0.1, 200.0, "Reflections");
    addProperty("Steps", &m_steps, 4, 128, "Reflections");
    addProperty("Thickness", &m_thickness, 0.0, 5.0, "Reflections");
    addProperty("Roughness", &m_roughness, 0.0, 1.0, "Reflections");
    addProperty("Fresnel", &m_fresnel, 0.0, 1.0, "Reflections");
    addProperty("Edge fade", &m_edgeFade, 0.0, 0.5, "Reflections");
    addProperty("Surface angle", &m_surfaceAngle, 0.0, 180.0, "Reflections");
    addProperty("Up direction", &m_upDirection, "Reflections");
    addProperty("Show reflections", &m_showReflections, "Reflections");
}
