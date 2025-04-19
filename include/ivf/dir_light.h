#pragma once

#include <ivf/light.h>
#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

namespace ivf {

class DirectionalLight : public Light {
private:
    glm::vec3 m_direction;

public:
    DirectionalLight();

    static std::shared_ptr<DirectionalLight> create();

    void setDirection(glm::vec3 direction);
    glm::vec3 direction();

    virtual glm::mat4 calculateLightSpaceMatrix(BoundingBox &sceneBBox) override;

    void apply() override;
};

typedef std::shared_ptr<DirectionalLight> DirectionalLightPtr;

} // namespace ivf
