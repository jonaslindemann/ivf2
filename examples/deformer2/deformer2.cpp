/**
 * @file deformer2.cpp
 * @brief Deformer example
 * @author Jonas Lindemann
 * @example deformer2.cpp
 * @ingroup deformer_examples
 *
 * This example demonstrates the use of multiple wave deformers and turbulence
 * to create a dynamic, animated surface effect on a plane mesh.
 */

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
#include <numbers>

#include <ivf/gl.h>
#include <ivf/nodes.h>

#include <ivf/deformable_mesh_node.h>
#include <ivf/deformable_primitive.h>
#include <ivf/wave_deformer.h>
#include <ivf/turbulence_deformer.h>

#include <ivfui/ui.h>
#include <ivfui/object_inspector.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    PlanePtr m_plane;
    std::shared_ptr<DeformablePrimitive<Plane>> m_deformable;

    std::vector<std::shared_ptr<WaveDeformer>> m_waves;
    std::shared_ptr<TurbulenceDeformer> m_surfaceTurbulence;

    ObjectInspectorPtr m_wave1Inspector;
    ObjectInspectorPtr m_wave2Inspector;
    ObjectInspectorPtr m_wave3Inspector;
    ObjectInspectorPtr m_wave4Inspector;
    ObjectInspectorPtr m_wave5Inspector;
    ObjectInspectorPtr m_surfaceTurbulenceInspector;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        // Create and add axis to the scene for orientation

        auto axis = Axis::create();
        this->add(axis);

        // Create some materials for potential use

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

        // Set mesh usage for dynamic updates

        mmDefaultMeshUsage(GL_DYNAMIC_DRAW);

        // Create a deformable plane primitive

        m_deformable = DeformablePrimitive<Plane>::create();
        m_deformable->primitive()->set(20.0, 20.0, 200.0, 200.0); // width, depth, rows, cols
        m_deformable->refresh();                                  // Update geometry and deformer setup
        m_deformable->setMaterial(whiteMaterial);
        m_deformable->setWireframe(false);

        // Create large wave deformers

        auto wave1 = WaveDeformer::create(0.2f, 0.8f, 1.2f, glm::vec3(1, 0, 0.3f), glm::vec3(0, 1, 0));
        auto wave2 = WaveDeformer::create(0.1f, 1.1f, 0.9f, glm::vec3(0.7f, 0, -0.7f), glm::vec3(0, 1, 0));

        // Create medium wave deformers
        
        auto wave3 = WaveDeformer::create(0.05f, 2.2f, 1.8f, glm::vec3(-0.5f, 0, 0.8f), glm::vec3(0, 1, 0));
        auto wave4 = WaveDeformer::create(0.03f, 2.8f, 2.2f, glm::vec3(0.9f, 0, 0.4f), glm::vec3(0, 1, 0));

        // Create small ripple deformer

        auto wave5 = WaveDeformer::create(0.02f, 4.5f, 3.1f, glm::vec3(0.3f, 0, -0.9f), glm::vec3(0, 1, 0));

        // Store all wave deformers in a vector for easy access

        m_waves = {wave1, wave2, wave3, wave4, wave5};

        // Add all wave deformers to the deformable mesh

        for (auto &wave : m_waves)
        {
            m_deformable->addDeformer(wave);
        }

        // Add a turbulence deformer for surface detail

        m_surfaceTurbulence = TurbulenceDeformer::create(10.0f, 0.01f, 4, 0.5f, 0.001f);
        m_deformable->addDeformer(m_surfaceTurbulence);

        // Add the deformable mesh to the scene

        this->add(m_deformable);

        // Create object inspectors for each wave and turbulence deformer

        m_wave1Inspector = ObjectInspector::create("Wave 1");
        m_wave1Inspector->setObject(wave1);
        m_wave1Inspector->setVisible(true);

        m_wave2Inspector = ObjectInspector::create("Wave 2");
        m_wave2Inspector->setObject(wave2);
        m_wave2Inspector->setVisible(true);

        m_wave3Inspector = ObjectInspector::create("Wave 3");
        m_wave3Inspector->setObject(wave3);
        m_wave3Inspector->setVisible(true);

        m_wave4Inspector = ObjectInspector::create("Wave 4");
        m_wave4Inspector->setObject(wave4);
        m_wave4Inspector->setVisible(true);

        m_wave5Inspector = ObjectInspector::create("Wave 5");
        m_wave5Inspector->setObject(wave5);
        m_wave5Inspector->setVisible(true);

        m_surfaceTurbulenceInspector = ObjectInspector::create("Surface Turbulence");
        m_surfaceTurbulenceInspector->setObject(m_surfaceTurbulence);
        m_surfaceTurbulenceInspector->setVisible(true);

        // Add all inspectors as UI windows

        this->addUiWindow(m_wave1Inspector);
        this->addUiWindow(m_wave2Inspector);
        this->addUiWindow(m_wave3Inspector);
        this->addUiWindow(m_wave4Inspector);
        this->addUiWindow(m_wave5Inspector);
        this->addUiWindow(m_surfaceTurbulenceInspector);

        // Set initial camera position for a good view of the scene
        
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
