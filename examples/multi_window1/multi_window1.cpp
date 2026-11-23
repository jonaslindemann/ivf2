/**
 * @file multi_window1.cpp
 * @brief Two windows, each with its own scene, lights and user interface.
 * @ingroup general_examples
 *
 * Every GLFWWindow owns an OpenGL context and an ivf::RenderContext holding the
 * managers for that context (shaders, lights, textures, ...). The window makes
 * its context current before calling onSetup/onUpdate/onDraw and before input
 * callbacks, so renderContext()->lights() - and the classic
 * LightManager::instance() - always refer to the window being processed.
 *
 * Changing the light in one window does not affect the other.
 *
 * The floor is one node shared by both scenes. It is created in main() before any
 * window (and OpenGL context) exists: meshes are uploaded when first drawn, and each
 * window gets its own GPU copy.
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;

class SceneWindow : public GLFWSceneWindow {
private:
    std::string m_name;
    glm::vec3 m_lightColor;
    bool m_spheres;
    DirectionalLightPtr m_light;
    NodePtr m_sharedNode;

public:
    SceneWindow(int w, int h, std::string title, glm::vec3 lightColor, bool spheres)
        : GLFWSceneWindow(w, h, title), m_name(title), m_lightColor(lightColor), m_spheres(spheres)
    {}

    static std::shared_ptr<SceneWindow> create(int w, int h, std::string title, glm::vec3 lightColor, bool spheres)
    {
        return std::make_shared<SceneWindow>(w, h, title, lightColor, spheres);
    }

    // A node also shown by other windows (added to the scene in onSetup)
    void setSharedNode(NodePtr node)
    {
        m_sharedNode = node;
    }

    int onSetup() override
    {
        // This window's lights - independent of the other window. Replace the
        // default white light from setupDefaults() with a coloured one.
        auto lights = renderContext()->lights();
        lights->clearLights();
        m_light = lights->addDirectionalLight();
        m_light->setDirection({-0.5f, -1.0f, -0.4f});
        m_light->setDiffuseColor(m_lightColor);
        m_light->setEnabled(true);
        lights->apply();

        auto material = Material::create();
        material->setDiffuseColor({0.8f, 0.8f, 0.8f, 1.0f});

        for (int i = -2; i <= 2; ++i)
            for (int j = -2; j <= 2; ++j)
            {
                TransformNodePtr node;
                if (m_spheres)
                    node = Sphere::create(0.6);
                else
                    node = Cube::create();
                node->setPos({i * 2.0f, 0.0f, j * 2.0f});
                node->setMaterial(material);
                add(node);
            }

        if (m_sharedNode)
            add(m_sharedNode);

        cameraManipulator()->setCameraPosition({0.0f, 8.0f, 12.0f});
        return 0;
    }

    void onDrawUi() override
    {
        // Each window also has its own ImGui context.
        ImGui::Begin(("Light - " + m_name).c_str());
        if (ImGui::ColorEdit3("Color", &m_lightColor.x))
        {
            m_light->setDiffuseColor(m_lightColor);
            renderContext()->lights()->apply();
        }
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

    // Built before any window exists; both windows draw it
    auto floorMaterial = Material::create();
    floorMaterial->setDiffuseColor({0.45f, 0.45f, 0.5f, 1.0f});
    auto floor = Box::create({11.0f, 0.2f, 11.0f});
    floor->setPos({0.0f, -0.8f, 0.0f});
    floor->setMaterial(floorMaterial);

    auto warm = SceneWindow::create(800, 600, "Window A (spheres)", {1.0f, 0.75f, 0.5f}, true);
    auto cool = SceneWindow::create(800, 600, "Window B (cubes)", {0.5f, 0.7f, 1.0f}, false);
    warm->setSharedNode(floor);
    cool->setSharedNode(floor);
    glfwSetWindowPos(warm->ref(), 80, 120);
    glfwSetWindowPos(cool->ref(), 920, 120);

    app->addWindow(warm);
    app->addWindow(cool);
    return app->loop();
}
