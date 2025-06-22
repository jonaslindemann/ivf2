/**
 * @file lighting1.cpp
 * @brief Lighting example
 * @author Jonas Lindemann
 * @example lighting1.cpp
 * @ingroup lighting_examples
 *
 * This example demonstrates the use of various light types in a 3D scene,
 * including point lights, directional lights, and spotlights.
 * It provides a user interface to control the properties of these lights
 * and visualize their effects on a simple scene with multiple spheres.
 * The scene includes a grid of spheres with random colors,
 * and allows toggling the visibility of different light controls.
 * The example also includes FPS display and an option to show the ImGui demo window.
 */

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    PointLightWindowPtr m_pointLightControl1;
    PointLightWindowPtr m_pointLightControl2;
    DirectionalLightWindowPtr m_dirLightControl;
    SpotLightWindowPtr m_spotLightControl;

    FpsWindowPtr m_fpsWindow;

    bool m_showDemoWindow = false;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Get the singleton instance of the LightManager

        auto lightMgr = LightManager::instance();

        // Remove any existing lights from the scene

        lightMgr->clearLights();

        // Add and configure the first point light

        auto pointLight1 = lightMgr->addPointLight();
        pointLight1->setEnabled(false);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.14, 0.07);
        pointLight1->setPosition(glm::vec3(2.0, 2.0, 2.0));

        // Add and configure the second point light

        auto pointLight2 = lightMgr->addPointLight();
        pointLight2->setEnabled(false);
        pointLight2->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight2->setAttenuation(1.0, 0.14, 0.07);
        pointLight2->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight2->setPosition(glm::vec3(-2.0, -2.0, -2.0));

        // Add and configure a directional light

        auto dirLight = lightMgr->addDirectionalLight();
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
        dirLight->setEnabled(false);

        // Add and configure a spotlight

        auto spotLight = lightMgr->addSpotLight();
        spotLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        spotLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        spotLight->setDirection(glm::vec3(0.0, 0.0, -1.0));
        spotLight->setPosition(glm::vec3(0.0, 0.0, 10.0));
        spotLight->setCutoff(12.0f, 13.0f);
        spotLight->setEnabled(true);

        // Apply the light settings to the scene

        lightMgr->apply();

        // Create an axis node for scene orientation

        auto axis = Axis::create();

        // Load a texture for use in the scene

        auto texture = Texture::create();
        texture->load("assets/planks.png");

        // Create a base sphere node to be instanced

        auto sphere = Sphere::create(0.15);

        // Create a 3D grid of spheres with random colors

        for (auto i = 0; i < 11; i++)
            for (auto j = 0; j < 11; j++)
                for (auto k = 0; k < 11; k++)
                {
                    if (true)
                    {
                        // Use instancing for efficiency

                        auto instSphere = InstanceNode::create();
                        instSphere->setNode(sphere);
                        instSphere->setPos(glm::vec3(-5.0 + i, -5.0 + j, -5.0 + k));

                        // Assign a random colored material to the instance

                        auto material = Material::create();
                        material->setDiffuseColor(
                            glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0f));
                        material->setShininess(40.0);

                        instSphere->setMaterial(material);
                        this->add(instSphere);
                    }
                    else
                    {
                        // (Unused branch) Create individual spheres instead of instances

                        auto sphere = Sphere::create(0.15);
                        sphere->setPos(glm::vec3(-5.0 + i, -5.0 + j, -5.0 + k));

                        auto material = Material::create();
                        material->setDiffuseColor(
                            glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0f));
                        material->setShininess(40.0);

                        sphere->setMaterial(material);
                        this->add(sphere);
                    }
                }

        // Add the axis to the scene

        this->add(axis);

        // Create UI controls for each light

        m_pointLightControl1 = PointLightWindow::create(pointLight1, "pointLight1");
        m_pointLightControl2 = PointLightWindow::create(pointLight2, "pointLight2");
        m_dirLightControl = DirectionalLightWindow::create(dirLight, "dirLight");
        m_spotLightControl = SpotLightWindow::create(spotLight, "spotLight");

        // Add the light control windows to the UI

        this->addUiWindow(m_pointLightControl1);
        this->addUiWindow(m_pointLightControl2);
        this->addUiWindow(m_dirLightControl);
        this->addUiWindow(m_spotLightControl);

        // Create and add an FPS display window

        m_fpsWindow = FpsWindow::create();

        this->addUiWindow(m_fpsWindow);

        // Return success

        return 0;
    }

    void onUpdateUi()
    {
        if ((m_pointLightControl1->isDirty()) || (m_pointLightControl2->isDirty()) || (m_dirLightControl->isDirty()) ||
            (m_spotLightControl->isDirty()))
            LightManager::instance()->apply();
    }
};

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = ExampleWindow::create(1280, 800, "Lighting");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
