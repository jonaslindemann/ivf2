#pragma once

#include <ivf/glbase.h>
#include <ivf/shadow_map.h>
#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

#include <string>

namespace ivf {

class Light : public GLBase {
private:
    glm::vec3 m_position;
    glm::vec3 m_diffuseColor;
    glm::vec3 m_specularColor;
    glm::vec3 m_ambientColor;
    bool m_enabled;
    int m_index;
    std::string m_lightArrayName;

    bool m_castsShadows{false};
    ShadowMapPtr m_shadowMap;

    size_t m_shadowMapWidth{2048};
    size_t m_shadowMapHeight{2048};

public:
    Light();

    static std::shared_ptr<Light> create();

    void setPosition(glm::vec3 pos);

    void setDiffuseColor(glm::vec3 color);
    void setSpecularColor(glm::vec3 color);
    void setAmbientColor(glm::vec3 color);

    void setIndex(int idx);

    void setLightArrayName(std::string name);
    std::string lightArrayName();

    glm::vec3 position();

    glm::vec3 diffuseColor();
    glm::vec3 specularColor();
    glm::vec3 ambientColor();

    void setEnabled(bool flag);
    bool enabled();

    int index();

    void setCastShadows(bool flag);
    bool castsShadows() const;

    void setShadowMapSize(size_t width, size_t height);

    void setShadowMap(ShadowMapPtr shadowMap);
    ShadowMapPtr shadowMap();

    void clearShadowMap();

    virtual glm::mat4 calculateLightSpaceMatrix(BoundingBox &sceneBBox);

    virtual void apply();
};

typedef std::shared_ptr<Light> LightPtr;

} // namespace ivf
