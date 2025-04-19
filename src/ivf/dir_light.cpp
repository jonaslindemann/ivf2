#include <ivf/dir_light.h>

#include <ivf/light_manager.h>
#include <ivf/shader_manager.h>

#include <sstream>
#include <iostream>
#include <string>

using namespace ivf;

DirectionalLight::DirectionalLight() : m_direction(glm::vec3(0.0, 0.0, 1.0))
{
    setLightArrayName("dirLights");
}

std::shared_ptr<DirectionalLight> ivf::DirectionalLight::create()
{
    return std::make_shared<DirectionalLight>();
}

void ivf::DirectionalLight::setDirection(glm::vec3 direction)
{
    m_direction = direction;
}

glm::vec3 ivf::DirectionalLight::direction()
{
    return m_direction;
}

glm::mat4 ivf::DirectionalLight::calculateLightSpaceMatrix(BoundingBox &sceneBBox)
{
    // Create orthographic projection for directional light
    glm::vec3 center = sceneBBox.center();
    glm::vec3 size = sceneBBox.size();
    float radius = glm::length(size) * 0.5f;

    // Ensure the light direction is normalized
    glm::vec3 lightDir = glm::normalize(m_direction);

    // Try to position the light at a considerable distance
    glm::vec3 lightPos = center - lightDir * (radius * 3.0f);

    // Use a stable up vector - if light is pointing straight down, use a different up
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(lightDir, up)) > 0.99f)
        up = glm::vec3(0.0f, 0.0f, 1.0f);

    // Light view matrix
    glm::mat4 lightView = glm::lookAt(lightPos, center, up);

    // Orthographic projection - use a more generous size to ensure coverage
    float nearPlane = 0.1f;
    float farPlane = radius * 10.0f;
    float orthoSize = radius * 1.5f;
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);

    return lightProjection * lightView;
}

void ivf::DirectionalLight::apply()
{
    std::string prefix;
    std::stringstream ss;

    ss << lightArrayName() << "[" << index() << "].";
    prefix = ss.str();

    ShaderManager::instance()->currentProgram()->use();
    ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "diffuseColor", diffuseColor());
    ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "specularColor", specularColor());
    ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "ambientColor", ambientColor());
    ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "position", position());
    ShaderManager::instance()->currentProgram()->uniformBool(prefix + "enabled", enabled());
    ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "direction", direction());
    ShaderManager::instance()->currentProgram()->uniformBool(prefix + "castShadows", castsShadows());
    ShaderManager::instance()->currentProgram()->uniformMat4(
        prefix + "lightSpaceMatrix", calculateLightSpaceMatrix(LightManager::instance()->sceneBoundingBox()));
}
