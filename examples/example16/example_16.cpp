#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <ivf/node_visitor.h>
#include <ivf/stock_shaders.h>
#include <ivf/blur_effect.h>
#include <ivf/spline_animation.h>
#include <ivf/keyframe_animation.h>
#include <ivf/model_loader.h>
#include <ivf/light_manager.h>
#include <ivf/shadow_map.h>

#include <ivf/tint_effect.h>
#include <ivf/filmgrain_effect.h>
#include <ivf/chromatic_effect.h>
#include <ivf/vignette_effect.h>
#include <ivf/bloom_effect.h>
#include <ivf/dithering_effect.h>
#include <ivf/pixelation_effect.h>

#include <ivfmath/spline.h>

#include <ivf/grid_layout.h>
#include <ivf/extent_visitor.h>

using namespace ivf;
using namespace ivfui;
using namespace ivfmath;

using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    CompositeNodePtr m_nodes;
    MaterialPtr m_material;
    RoundedBoxPtr m_box;
    DirectionalLightPtr m_dirLight0;
    CompositeNodePtr m_grid;

    int m_debugShadow{0};

    float m_a1{0.0f}, m_a2{0.0f}, m_a3{0.0f};

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        auto lightManager = LightManager::instance();
        lightManager->clearLights();
        lightManager->setUseShadows(true);
        lightManager->setAutoCalcBBox(false);
        lightManager->setSceneBoundingBox(glm::vec3(-30.0, -30.0, -20.0), glm::vec3(30.0, 30.0, 30.0));

        m_dirLight0 = lightManager->addDirectionalLight();
        m_dirLight0->setAmbientColor(glm::vec3(0.3, 0.3, 0.3));
        m_dirLight0->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        m_dirLight0->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        m_dirLight0->setDirection(glm::vec3(-0.2, -1.0, -0.2));
        m_dirLight0->setEnabled(true);
        m_dirLight0->setCastShadows(true);
        m_dirLight0->setShadowMapSize(4096, 4096);
        m_dirLight0->setShadowStrength(0.3f); // Set shadow strength (0.0 to 1.0) for the directional light

        lightManager->apply();

        auto axis = Axis::create();

        auto planeMaterial = Material::create();
        planeMaterial->setDiffuseColor(glm::vec4(0.8, 0.8, 0.8, 1.0));

        auto plane = Plane::create();
        plane->set(20.0, 20.0, 10, 10);
        plane->refresh();
        plane->setMaterial(planeMaterial);

        auto boxMaterial = Material::create();
        boxMaterial->setDiffuseColor(glm::vec4(0.8, 0.8, 0.0, 1.0));

        m_grid = CompositeNode::create();

        for (auto row = -3; row <= 3; row++)
        {
            for (auto col = -3; col <= 3; col++)
            {
                for (auto stack = -3; stack <= 3; stack++)
                {
                    auto box = RoundedBox::create();
                    box->setPos(glm::vec3(row * 2.0, stack * 2.0, col * 2.0));
                    box->setSize(0.3, 0.3, 0.3);
                    box->setSegments(5, 5, 5);
                    box->setRadius(0.05);
                    box->setSlices(5);
                    box->refresh();
                    box->setMaterial(boxMaterial);
                    m_grid->add(box);
                }
            }
        }

        m_grid->setPos(glm::vec3(0.0, 12.0, 0.0));

        this->add(m_grid);

        this->add(plane);

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0, 8.0, 70.0));

        return 0;
    }

    virtual void onUpdate()
    {
        m_a1 += 0.1;
        m_a2 += 0.1;
        m_a3 += 0.1;

        // |
        // |
        // |
        // o-------

        m_grid->setEulerAngles(m_a1, m_a2, m_a3);
    }

    virtual void onKey(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            this->close();
        }
        else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            LightManager::instance()->setUseShadows(!LightManager::instance()->useShadows());
        }
        else if (key == GLFW_KEY_D && action == GLFW_PRESS)
        {
            m_debugShadow++;
            LightManager::instance()->setDebugShadow(m_debugShadow);

            if (m_debugShadow > 4)
                m_debugShadow = 0;
        }
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

    auto window = ExampleWindow::create(800, 800, "Example 6");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
