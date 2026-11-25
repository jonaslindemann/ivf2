#include <ivf/light.h>
#include <glm/glm.hpp>

#include <algorithm>

using namespace ivf;

ivf::Light::Light()
    : m_position(glm::vec3(0.0, 0.0, 0.0)), m_diffuseColor(glm::vec3(1.0, 1.0, 1.0)),
      m_specularColor(glm::vec3(1.0, 1.0, 1.0)), m_ambientColor(glm::vec3(0.2, 0.2, 0.2)), m_enabled(true), m_index(-1),
      m_lightArrayName("Lights")
{}

std::shared_ptr<Light> ivf::Light::create()
{
    return std::make_shared<Light>();
}

void ivf::Light::setPosition(glm::vec3 pos)
{
    m_position = pos;
}

void ivf::Light::setDiffuseColor(glm::vec3 color)
{
    m_diffuseColor = color;
}

void ivf::Light::setSpecularColor(glm::vec3 color)
{
    m_specularColor = color;
}

void ivf::Light::setAmbientColor(glm::vec3 color)
{
    m_ambientColor = color;
}

void ivf::Light::setIndex(int idx)
{
    m_index = idx;
}

void ivf::Light::setLightArrayName(std::string name)
{
    m_lightArrayName = name;
}

std::string ivf::Light::lightArrayName() const
{
    return m_lightArrayName;
}

int ivf::Light::index() const
{
    return m_index;
}

void ivf::Light::setCastShadows(bool flag)
{
    // The shadow map is created by LightManager::renderShadowMaps() when it is first needed
    m_castsShadows = flag;
}

bool ivf::Light::castsShadows() const
{
    return m_castsShadows;
}

void ivf::Light::setShadowMapSize(size_t width, size_t height)
{
    // Applied by LightManager::renderShadowMaps() (the map's size also depends on the light type)
    m_shadowMapWidth = std::max<size_t>(width, 16);
    m_shadowMapHeight = std::max<size_t>(height, 16);
}

size_t ivf::Light::shadowMapWidth() const
{
    return m_shadowMapWidth;
}

size_t ivf::Light::shadowMapHeight() const
{
    return m_shadowMapHeight;
}

void ivf::Light::setShadowFilter(ShadowFilter filter)
{
    m_shadowFilter = filter;
}

ShadowFilter ivf::Light::shadowFilter() const
{
    return m_shadowFilter;
}

void ivf::Light::setShadowFilterRadius(int radius)
{
    m_shadowFilterRadius = std::clamp(radius, 0, 4);
}

int ivf::Light::shadowFilterRadius() const
{
    return m_shadowFilterRadius;
}

void ivf::Light::setShadowSoftness(float softness)
{
    m_shadowSoftness = std::max(softness, 0.0f);
}

float ivf::Light::shadowSoftness() const
{
    return m_shadowSoftness;
}

void ivf::Light::setShadowBias(float bias)
{
    m_shadowBias = std::max(bias, 0.0f);
}

float ivf::Light::shadowBias() const
{
    return m_shadowBias;
}

void ivf::Light::setShadowNormalBias(float texels)
{
    m_shadowNormalBias = std::max(texels, 0.0f);
}

float ivf::Light::shadowNormalBias() const
{
    return m_shadowNormalBias;
}

void ivf::Light::setShadowMap(ShadowMapPtr shadowMap)
{
    m_shadowMap = shadowMap;
}

ShadowMapPtr ivf::Light::shadowMap() const
{
    return m_shadowMap;
}

void ivf::Light::clearShadowMap()
{
    if (m_shadowMap)
        m_shadowMap = nullptr;
}

glm::mat4 ivf::Light::calculateLightSpaceMatrix(BoundingBox &/*sceneBBox*/)
{
    return glm::mat4();
}

void ivf::Light::setShadowStrength(float strength)
{
    m_shadowStrength = strength;
}

float ivf::Light::shadowStrength() const
{
    return m_shadowStrength;
}

void ivf::Light::apply()
{}

glm::vec3 ivf::Light::position() const
{
    return m_position;
}

glm::vec3 ivf::Light::diffuseColor() const
{
    return m_diffuseColor;
}

glm::vec3 ivf::Light::specularColor() const
{
    return m_specularColor;
}

glm::vec3 ivf::Light::ambientColor() const
{
    return m_ambientColor;
}

void ivf::Light::setEnabled(bool flag)
{
    m_enabled = flag;
}

bool ivf::Light::enabled() const
{
    return m_enabled;
}
