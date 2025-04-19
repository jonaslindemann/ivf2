#pragma once

#include <ivf/light.h>

namespace ivf {

class SpotLight : public Light {
private:
    glm::vec3 m_direction;
    float m_cutoff;
    float m_outerCutoff;
    float m_constAttenuation;
    float m_linearAttenuation;
    float m_quadraticAttenuation;

public:
    SpotLight();

    static std::shared_ptr<SpotLight> create();

    void setDirection(glm::vec3 direction);
    void setAttenuation(float cnst, float linear, float quadratic);
    void setCutoff(float inner, float outer);

    glm::vec3 direction();

    float constAttenuation();
    float linearAttenutation();
    float quadraticAttenuation();

    float innerCutoff();
    float outerCutoff();

    void apply();
};

typedef std::shared_ptr<SpotLight> SpotLightPtr;

} // namespace ivf
