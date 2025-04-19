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
        // this->setSelectionEnabled(true);
        // this->setRenderToTexture(true);
        // this->enableHeadlight();

        auto shadowMap = ShadowMap::create(2048, 2048);

        auto lightManager = LightManager::instance();
        lightManager->clearLights();
        lightManager->setUseShadows(true);
        lightManager->setAutoCalcBBox(false);
        lightManager->setSceneBoundingBox(glm::vec3(-10.0, -10.0, -10.0), glm::vec3(10.0, 10.0, 10.0));

        auto dirLight = lightManager->addDirectionalLight();
        dirLight->setAmbientColor(glm::vec3(0.3, 0.3, 0.3));
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -2.0, -3.0));
        dirLight->setEnabled(true);
        dirLight->setCastShadows(true);
        dirLight->setShadowMap(shadowMap);
        lightManager->apply();

        auto axis = Axis::create();

        auto planeMaterial = Material::create();
        planeMaterial->setDiffuseColor(glm::vec4(0.8, 0.8, 0.8, 1.0));

        auto plane = Plane::create();
        plane->set(10.0, 10.0, 10, 10);
        plane->refresh();
        plane->setMaterial(planeMaterial);

        auto boxMaterial = Material::create();
        boxMaterial->setDiffuseColor(glm::vec4(0.8, 0.8, 0.0, 1.0));

        m_box = RoundedBox::create();
        m_box->setPos(glm::vec3(0.0, 4.0, 0.0));
        m_box->setSize(2.0, 2.0, 2.0);
        m_box->setSegments(10, 10, 10);
        m_box->setRadius(0.5);
        m_box->setSlices(10);
        m_box->refresh();
        m_box->setMaterial(boxMaterial);

        this->add(plane);
        this->add(m_box);

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0, 8.0, 40.0));
        // this->cameraManipulator()->setCameraTarget(bbox.center());

        return 0;
    }

    virtual void onUpdate()
    {
        m_a1 += 0.1;
        m_a2 += 0.1;
        m_a3 += 0.1;

        m_box->setEulerAngles(m_a1, m_a2, m_a3);
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
