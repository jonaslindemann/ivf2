#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
#include <numbers>

#include <ivf/gl.h>
#include <ivf/nodes.h>

#include <ivf/twist_deformer.h>
#include <ivf/bend_deformer.h>
#include <ivf/deformable_mesh_node.h>
#include <ivf/deformable_primitive.h>
#include <ivf/wave_deformer.h>
#include <ivf/turbulence_deformer.h>

#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    PlanePtr m_plane;
    std::shared_ptr<DeformablePrimitive<Plane>> m_deformable;

    std::vector<std::shared_ptr<WaveDeformer>> m_waves;
    std::shared_ptr<TurbulenceDeformer> m_surfaceTurbulence;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        auto axis = Axis::create();

        this->add(axis);

        auto redMaterial = Material::create();
        redMaterial->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        auto greenMaterial = Material::create();
        greenMaterial->setDiffuseColor(glm::vec4(0.0, 1.0, 0.0, 1.0));

        auto blueMaterial = Material::create();
        blueMaterial->setDiffuseColor(glm::vec4(0.0, 0.0, 1.0, 1.0));

        auto yellowMaterial = Material::create();
        yellowMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto whiteMaterial = Material::create();
        whiteMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

        mmDefaultMeshUsage(GL_DYNAMIC_DRAW);

        m_deformable = DeformablePrimitive<Plane>::create();
        m_deformable->primitive()->set(20.0, 20.0, 200.0, 200.0); // Set width, depth, rows, cols
        m_deformable->refresh();                                  // Update geometry and deformer setup
        m_deformable->setMaterial(whiteMaterial);
        m_deformable->setWireframe(false);

        // Large waves
        auto wave1 = WaveDeformer::create(0.2f, 0.8f, 1.2f, glm::vec3(1, 0, 0.3f), glm::vec3(0, 1, 0));
        auto wave2 = WaveDeformer::create(0.1f, 1.1f, 0.9f, glm::vec3(0.7f, 0, -0.7f), glm::vec3(0, 1, 0));

        // Medium waves
        auto wave3 = WaveDeformer::create(0.05f, 2.2f, 1.8f, glm::vec3(-0.5f, 0, 0.8f), glm::vec3(0, 1, 0));
        auto wave4 = WaveDeformer::create(0.03f, 2.8f, 2.2f, glm::vec3(0.9f, 0, 0.4f), glm::vec3(0, 1, 0));

        // Small ripples
        auto wave5 = WaveDeformer::create(0.02f, 4.5f, 3.1f, glm::vec3(0.3f, 0, -0.9f), glm::vec3(0, 1, 0));

        m_waves = {wave1, wave2, wave3, wave4, wave5};

        for (auto &wave : m_waves)
        {
            m_deformable->addDeformer(wave);
        }

        // Add surface turbulence
        m_surfaceTurbulence = TurbulenceDeformer::create(3.0f, 0.05f, 4, 0.7f, 0.01f);
        // m_deformable->addDeformer(m_surfaceTurbulence);

        this->add(m_deformable);

        this->cameraManipulator()->setCameraPosition(glm::vec3(0, 5, 20));

        return 0;
    }

    virtual void onUpdate()
    {
        float time = this->elapsedTime();

        // Update all wave systems
        for (auto &wave : m_waves)
        {
            wave->setTime(time);
        }

        m_surfaceTurbulence->setTime(time);

        m_deformable->applyDeformers();
    }

    virtual void onKey(int key, int scancode, int action, int mods) override
    {
        if (key == GLFW_KEY_W && action == GLFW_PRESS)
        {}
        else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            this->close();
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

    auto window = ExampleWindow::create(800, 800, "Animation");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
