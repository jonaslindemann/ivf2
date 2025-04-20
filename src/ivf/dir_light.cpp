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
    // Ensure the direction is normalized
    if (glm::length(direction) > 0.0f)
        m_direction = glm::normalize(direction);
    else
        std::cerr << "Warning: Directional light direction cannot be zero." << std::endl;
}

glm::vec3 ivf::DirectionalLight::direction()
{
    return m_direction;
}

glm::mat4 ivf::DirectionalLight::calculateLightSpaceMatrix(BoundingBox &sceneBBox)
{
    // Get scene properties
    glm::vec3 center = sceneBBox.center();
    glm::vec3 size = sceneBBox.size();
    float radius = glm::length(size) * 0.5f;

    // Ensure the light direction is normalized
    glm::vec3 lightDir = glm::normalize(m_direction);

    // Position the light at a distance from the scene center
    glm::vec3 lightPos = center - lightDir * (radius * 4.0f);

    // Create a stable view basis that works for any light direction
    glm::vec3 right, up;

    // Find the smallest component of the light direction
    float absX = std::abs(lightDir.x);
    float absY = std::abs(lightDir.y);
    float absZ = std::abs(lightDir.z);

    if (absX <= absY && absX <= absZ)
        right = glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), lightDir));
    else if (absY <= absX && absY <= absZ)
        right = glm::normalize(glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), lightDir));
    else
        right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), lightDir));

    up = glm::normalize(glm::cross(lightDir, right));

    // Create view matrix
    glm::mat4 lightView = glm::lookAt(lightPos, center, up);

    // Create orthographic projection with a bit more space
    float orthoSize = radius * 2.5f;
    glm::mat4 lightProjection =
        glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, radius * 12.0f); // Use a larger depth range

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
    // ShaderManager::instance()->currentProgram()->uniformMat4(
    //     "lightSpaceMatrix", calculateLightSpaceMatrix(LightManager::instance()->sceneBoundingBox()));
}
