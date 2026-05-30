/**
 * @file bump1.cpp
 * @brief Normal-map (bump mapping) example using BumpMaterial.
 * @author Jonas Lindemann
 * @example bump1.cpp
 * @ingroup texture_examples
 *
 * Demonstrates the BumpMaterial class, which adds per-fragment normal-map
 * support to the standard Phong shading model using derivative-based TBN
 * reconstruction (no extra vertex attributes required).
 *
 * The scene shows two large spheres side-by-side:
 *  - Left:  plain Material (standard Phong, no bump)
 *  - Right: BumpMaterial with a normal map
 *
 * Both spheres share the same diffuse texture (planks.png) so the bump
 * effect is clearly visible.  A point light orbits the spheres so you can
 * watch surface detail emerge and disappear as the light angle changes.
 *
 * Normal map file
 * ---------------
 * The example looks for "assets/planks_normal.png".  If it is not present
 * the right sphere still renders correctly — just with flat Phong shading.
 * You can generate a normal map from any diffuse texture with tools such as
 * GIMP (Filters → Generic → Normal Map) or the free online converter at
 * cpetry.github.io/NormalMap-Online.
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;

class BumpWindow : public GLFWSceneWindow {
private:
    PointLightPtr  m_light;
    BumpMaterialPtr m_bumpMat;
    TexturePtr     m_normalMap;

    float m_lightAngle{0.0f};
    float m_lightRadius{6.0f};
    float m_lightHeight{4.0f};
    float m_lightSpeed{0.6f};
    bool  m_animateLight{true};
    bool  m_showNormalMap{true};

public:
    BumpWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<BumpWindow> create(int w, int h, std::string title)
    {
        return std::make_shared<BumpWindow>(w, h, title);
    }

    int onSetup() override
    {
        // Register the bump shader (basic + PBR are already loaded by GLFWSceneWindow).
        ShaderManager::instance()->loadBumpShader();

        // Enable texture sampling in the basic shader (used by the left sphere).
        // The bump shader gets useTexture forwarded from the material in BumpMaterial::apply().
        auto texMgr = TextureManager::create();
        texMgr->setUseTexture(true);
        texMgr->apply();

        // Single orbiting point light for maximum bump-map impact.
        auto lightMgr = LightManager::instance();
        lightMgr->clearLights();

        m_light = lightMgr->addPointLight();
        m_light->setEnabled(true);
        m_light->setDiffuseColor(glm::vec3(1.0f, 0.95f, 0.85f));
        m_light->setSpecularColor(glm::vec3(1.0f, 0.95f, 0.85f));
        m_light->setAmbientColor(glm::vec3(0.06f));
        m_light->setAttenuation(1.0f, 0.0f, 0.0f);
        m_light->setPosition(glm::vec3(m_lightRadius, m_lightHeight, 0.0f));
        lightMgr->apply();

        // Shared diffuse texture.
        auto diffuseTex = Texture::create();
        diffuseTex->load("assets/brick.png");

        // ---- Left sphere: standard Phong material ----------------------------
        auto plainMat = Material::create();
        plainMat->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        plainMat->setSpecularColor(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
        plainMat->setAmbientColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
        plainMat->setShininess(32.0f);
        plainMat->setUseTexture(true);

        auto sphereLeft = Sphere::create(1.8f, 64, 64);
        sphereLeft->setMaterial(plainMat);
        sphereLeft->setTexture(diffuseTex);
        sphereLeft->setPos(glm::vec3(-2.5f, 0.0f, 0.0f));
        add(sphereLeft);

        // ---- Right sphere: bump material ------------------------------------
        m_bumpMat = BumpMaterial::create();
        m_bumpMat->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_bumpMat->setSpecularColor(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
        m_bumpMat->setAmbientColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
        m_bumpMat->setShininess(32.0f);
        m_bumpMat->setUseTexture(true);

        // Try to load a normal map. If the file is absent the sphere still
        // renders — just without normal-map perturbation.
        m_normalMap = Texture::create();
        if (m_normalMap->load("assets/brick_normal.png"))
            m_bumpMat->setNormalMap(m_normalMap);

        auto sphereRight = Sphere::create(1.8f, 64, 64);
        sphereRight->setMaterial(m_bumpMat);
        sphereRight->setTexture(diffuseTex);
        sphereRight->setPos(glm::vec3(2.5f, 0.0f, 0.0f));
        add(sphereRight);

        // Position camera.
        cameraManipulator()->setCameraPosition(glm::vec3(0.0f, 0.0f, 10.0f));

        return 0;
    }

    void onUpdate() override
    {
        // Animate the point light around the scene.
        if (m_animateLight)
        {
            m_lightAngle += m_lightSpeed * float(frameTime());
            float x = m_lightRadius * cosf(m_lightAngle);
            float z = m_lightRadius * sinf(m_lightAngle);
            m_light->setPosition(glm::vec3(x, m_lightHeight, z));
            LightManager::instance()->apply();
        }
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_FirstUseEver);
        ImGui::Begin("Bump Mapping");

        ImGui::Text("Left sphere:  standard Phong");
        ImGui::Text("Right sphere: BumpMaterial");

        bool hasNormalMap = (m_bumpMat->normalMap() != nullptr);
        if (hasNormalMap)
        {
            ImGui::Separator();
            if (ImGui::Checkbox("Normal map enabled", &m_showNormalMap))
                m_bumpMat->setNormalMap(m_showNormalMap ? m_normalMap : nullptr);
        }
        else
        {
            ImGui::Separator();
            ImGui::TextWrapped("No normal map loaded.\nPlace assets/planks_normal.png\nto see the full bump effect.");
        }

        ImGui::Separator();
        ImGui::Checkbox("Animate light", &m_animateLight);
        ImGui::SliderFloat("Speed", &m_lightSpeed, 0.1f, 3.0f);
        ImGui::SliderFloat("Height", &m_lightHeight, -5.0f, 8.0f);

        ImGui::End();
    }

    void onKey(int key, int /*scancode*/, int action, int /*mods*/) override
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            close();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 0);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto win = BumpWindow::create(1280, 800, "Bump Mapping");
    win->maximize();

    app->addWindow(win);
    return app->loop();
}
