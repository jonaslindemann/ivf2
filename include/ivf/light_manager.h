#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <ivf/dir_light.h>
#include <ivf/point_light.h>
#include <ivf/spot_light.h>
#include <ivf/composite_node.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

class LightManager {
private:
    bool m_useLighting;
    bool m_useVertexColors;
    bool m_savedState;

    GLint m_useLightingId;
    GLint m_useVertexColorsId;
    GLint m_useTextureId;

    GLint m_pointLightCountId;
    GLint m_directionalLightCountId;
    GLint m_spotLightCountId;

    GLint m_diffuseColorId;
    GLint m_specularColorId;
    GLint m_ambientColorId;
    GLint m_shininessId;
    GLint m_alphaId;

    std::vector<PointLightPtr> m_pointLights;
    std::vector<DirectionalLightPtr> m_dirLights;
    std::vector<SpotLightPtr> m_spotLights;

    // Shadow mapping

    bool m_useShadows{false};
    GLint m_useShadowsId;
    GLint m_shadowMapId;
    GLint m_lightSpaceMatrixId;
    bool m_autoCalcBBox{true};
    BoundingBox m_sceneBBox;
    int m_debugShadow{0};

    LightManager();
    static LightManager *m_instance;

public:
    static LightManager *instance()
    {
        if (!m_instance)
            m_instance = new LightManager();

        return m_instance;
    }

    static LightManager *create()
    {
        return instance();
    }

    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    PointLightPtr addPointLight();
    DirectionalLightPtr addDirectionalLight();
    SpotLightPtr addSpotLight();

    PointLightPtr getPointLight(int idx);
    DirectionalLightPtr getDirectionalLight(int idx);
    SpotLightPtr getSpotLight(int idx);

    size_t pointLightCount();
    size_t dirLightCount();
    size_t spotLightCount();

    void clearPointLights();
    void clearDirectionalLights();
    void clearSpotLights();
    void clearLights();

    void setUseLighting(bool flag);
    void setUseVertexColors(bool flag);
    bool useLighting();
    void enableLighting();
    void disableLighting();

    void setupDefaultColors();

    void apply();

    void renderShadowMaps(CompositeNodePtr scene);

    void setDiffuseColor(glm::vec3 color);
    void setDiffuseColor(glm::vec4 color);
    void setDiffuseColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);

    void setSpecularColor(glm::vec3 color);
    void setSpecularColor(glm::vec4 color);
    void setSpecularColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);

    void setAmbientColor(glm::vec3 color);
    void setAmbientColor(glm::vec4 color);
    void setAmbientColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);

    void setShininess(float shininess);
    void setAlpha(float alpha);

    void setUseShadows(bool flag);
    bool useShadows() const;

    void setAutoCalcBBox(bool flag);
    bool autoCalcBBox() const;

    void setDebugShadow(int flag);
    int debugShadow() const;

    void setSceneBoundingBox(BoundingBox &bbox);
    void setSceneBoundingBox(glm::vec3 min, glm::vec3 max);
    BoundingBox sceneBoundingBox() const;
    BoundingBox &sceneBoundingBox();

    void saveState();
    void restoreState();
};

typedef LightManager *LightManagerPtr;
}; // namespace ivf
