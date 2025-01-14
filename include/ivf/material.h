#pragma once

#include <ivf/glbase.h>

#include <glm/glm.hpp>

namespace ivf {

class Material : public GLBase {
private:
    bool m_useVertexColor;
    bool m_useLighting;
    bool m_useTexture;
    glm::vec4 m_diffuseColor;
    glm::vec4 m_specularColor;
    glm::vec4 m_ambientColor;
    float m_alpha;
    float m_shininess;

public:
    Material();

    static std::shared_ptr<Material> create();

    void setDiffuseColor(glm::vec4 color);
    void setSpecularColor(glm::vec4 color);
    void setAmbientColor(glm::vec4 color);
    void setUseTexture(bool flag);
    void setUseVertexColor(bool flag);
    void setShininess(float shininess);
    void setUseLighting(bool flag);
    void setAlpha(float alpha);
    bool useLighting() const;
    bool useTexture() const;
    bool useVertexColor() const;
    glm::vec4 diffuseColor() const;
    glm::vec4 specularColor() const;
    glm::vec4 ambientColor() const;
    float shininess() const;
    float alpha() const;

    void apply();
};

typedef std::shared_ptr<Material> MaterialPtr;

} // namespace ivf
