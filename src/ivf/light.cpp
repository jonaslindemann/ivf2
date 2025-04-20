#include <ivf/light.h>
#include <glm/glm.hpp>

using namespace ivf;

ivf::Light::Light()
    : m_position(glm::vec3(0.0, 0.0, 0.0)), m_diffuseColor(glm::vec3(1.0, 1.0, 1.0)),
      m_ambientColor(glm::vec3(0.2, 0.2, 0.2)), m_specularColor(glm::vec3(1.0, 1.0, 1.0)), m_enabled(true), m_index(-1),
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

std::string ivf::Light::lightArrayName()
{
    return m_lightArrayName;
}

int ivf::Light::index()
{
    return m_index;
}

void ivf::Light::setCastShadows(bool flag)
{
    m_castsShadows = flag;

    if (m_castsShadows)
    {
        // Create shadow map if it doesn't exist

        if (m_shadowMap == nullptr)
            m_shadowMap = ShadowMap::create(2048, 2048);
    }
}

bool ivf::Light::castsShadows() const
{
    return m_castsShadows;
}

void ivf::Light::setShadowMapSize(size_t width, size_t height)
{
    m_shadowMapWidth = width;
    m_shadowMapHeight = height;
    if (m_shadowMap)
        m_shadowMap->resize(m_shadowMapWidth, m_shadowMapHeight);
}

void ivf::Light::setShadowMap(ShadowMapPtr shadowMap)
{
    m_shadowMap = shadowMap;
}

ShadowMapPtr ivf::Light::shadowMap()
{
    return m_shadowMap;
}

void ivf::Light::clearShadowMap()
{
    if (m_shadowMap)
        m_shadowMap = nullptr;
}

glm::mat4 ivf::Light::calculateLightSpaceMatrix(BoundingBox &sceneBBox)
{
    return glm::mat4();
}

void ivf::Light::apply()
{}

glm::vec3 ivf::Light::position()
{
    return m_position;
}

glm::vec3 ivf::Light::diffuseColor()
{
    return m_diffuseColor;
}

glm::vec3 ivf::Light::specularColor()
{
    return m_diffuseColor;
}

glm::vec3 ivf::Light::ambientColor()
{
    return m_ambientColor;
}

void ivf::Light::setEnabled(bool flag)
{
    m_enabled = flag;
}

bool ivf::Light::enabled()
{
    return m_enabled;
}
