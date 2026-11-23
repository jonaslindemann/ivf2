/**
 * @file transparency1.cpp
 * @brief Transparent materials and the render queue's draw order.
 * @ingroup general_examples
 *
 * Demonstrates:
 *  - Materials with alpha < 1: drawn after the opaque objects, back to front, with alpha
 *    blending (no extra setup - the render queue sorts every frame)
 *  - A transparent material set on a group, inherited by its children
 *  - A glass box around an opaque object, and an additive GlowMaterial orb
 *  - The order nodes are added in does not matter: the transparent spheres are added
 *    before the opaque pillar they surround
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/glow_material.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <cmath>

using namespace ivf;
using namespace ivfui;

class TransparencyWindow : public GLFWSceneWindow {
private:
    CompositeNodePtr m_ring;
    std::vector<MaterialPtr> m_ringMaterials;
    MaterialPtr m_glassMaterial;
    float m_ringAlpha{0.45f};
    float m_glassAlpha{0.25f};
    bool m_rotate{true};

public:
    TransparencyWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title)
    {}

    static std::shared_ptr<TransparencyWindow> create(int w, int h, std::string title)
    {
        return std::make_shared<TransparencyWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto light = renderContext()->lights()->addDirectionalLight();
        light->setDiffuseColor({1.0f, 0.95f, 0.9f});
        light->setDirection({-0.4f, -1.0f, -0.6f});
        light->setEnabled(true);

        // A ring of transparent spheres, added first: the queue still draws them after
        // the opaque pillar in the middle, back to front
        m_ring = CompositeNode::create();
        const glm::vec3 colors[] = {{1.0f, 0.3f, 0.3f}, {0.3f, 1.0f, 0.4f}, {0.3f, 0.5f, 1.0f},
                                    {1.0f, 0.9f, 0.3f}, {0.9f, 0.3f, 1.0f}, {0.3f, 1.0f, 1.0f}};
        for (int i = 0; i < 6; ++i)
        {
            auto material = Material::create();
            material->setDiffuseColor(glm::vec4(colors[i], 1.0f));
            material->setAlpha(m_ringAlpha);
            m_ringMaterials.push_back(material);

            const float angle = float(i) / 6.0f * 2.0f * 3.14159265f;
            auto sphere = Sphere::create(0.9);
            sphere->setPos({2.6f * std::cos(angle), 1.2f, 2.6f * std::sin(angle)});
            sphere->setMaterial(material);
            m_ring->add(sphere);
        }
        add(m_ring);

        // Opaque objects
        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.35f, 0.37f, 0.4f, 1.0f});
        auto ground = Plane::create(20.0, 20.0, 4, 4);
        ground->setMaterial(groundMaterial);
        add(ground);

        auto pillarMaterial = Material::create();
        pillarMaterial->setDiffuseColor({0.85f, 0.75f, 0.55f, 1.0f});
        auto pillar = Box::create({0.4f, 1.6f, 0.4f});
        pillar->setPos({0.0f, 1.6f, 0.0f});
        pillar->setMaterial(pillarMaterial);
        add(pillar);

        // A glass box around an opaque sphere
        auto coreMaterial = Material::create();
        coreMaterial->setDiffuseColor({0.9f, 0.2f, 0.2f, 1.0f});
        auto core = Sphere::create(0.5);
        core->setPos({-4.5f, 1.0f, -1.5f});
        core->setMaterial(coreMaterial);
        add(core);

        m_glassMaterial = Material::create();
        m_glassMaterial->setDiffuseColor({0.7f, 0.85f, 1.0f, 1.0f});
        m_glassMaterial->setAlpha(m_glassAlpha);
        auto glass = Box::create({0.9f, 0.9f, 0.9f});
        glass->setPos({-4.5f, 1.0f, -1.5f});
        glass->setMaterial(m_glassMaterial);
        add(glass);

        // A group whose material is inherited by its children (none has its own)
        auto tintMaterial = Material::create();
        tintMaterial->setDiffuseColor({0.2f, 0.9f, 0.5f, 1.0f});
        tintMaterial->setAlpha(0.35f);
        auto group = CompositeNode::create();
        group->setMaterial(tintMaterial);
        for (int i = 0; i < 3; ++i)
        {
            auto cube = Cube::create();
            cube->setPos({4.5f, 0.6f + i * 1.1f, -1.0f + i * 0.4f});
            group->add(cube);
        }
        add(group);

        // An additive glow orb (blended materials are transparent too)
        auto glow = GlowMaterial::create({1.0f, 0.6f, 0.2f}, 1.5f);
        if (!glow)
            return -1;
        auto orb = Sphere::create(0.7);
        orb->setPos({0.0f, 4.2f, 0.0f});
        orb->setMaterial(glow);
        add(orb);

        cameraManipulator()->setCameraPosition({0.0f, 5.0f, 11.0f});
        cameraManipulator()->setCameraTarget({0.0f, 1.2f, 0.0f});
        return 0;
    }

    void onUpdate() override
    {
        if (m_rotate)
            m_ring->setEulerAngles(0.0f, float(elapsedTime()) * 20.0f, 0.0f);

        for (auto &material : m_ringMaterials)
            material->setAlpha(m_ringAlpha);
        m_glassMaterial->setAlpha(m_glassAlpha);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({300, 140}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Transparency");
        ImGui::SliderFloat("Ring alpha", &m_ringAlpha, 0.0f, 1.0f);
        ImGui::SliderFloat("Glass alpha", &m_glassAlpha, 0.0f, 1.0f);
        ImGui::Checkbox("Rotate ring", &m_rotate);
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto win = TransparencyWindow::create(1280, 720, "Transparency");
    app->addWindow(win);
    return app->loop();
}
