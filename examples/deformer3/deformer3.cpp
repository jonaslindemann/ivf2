/**
 * @file deformer3.cpp
 * @brief Deformer example
 * @author Jonas Lindemann
 * @example deformer3.cpp
 * @ingroup deformer_examples
 *
 * This example demonstrates the use of various random deformers
 * to manipulate a 3D mesh, including uniform noise, Gaussian noise,
 */

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
#include <ivf/random_deformer.h>

#include <ivfui/ui.h>
#include <ivfui/object_inspector.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    std::shared_ptr<DeformablePrimitive<Plane>> m_defPlane;
    std::shared_ptr<DeformablePrimitive<Sphere>> m_defSphere;
    std::shared_ptr<DeformablePrimitive<CappedCylinder>> m_defCylinder;

    RandomDeformerPtr m_uniformNoise;
    RandomDeformerPtr m_gaussianNoise;
    RandomDeformerPtr m_perlinNoise;

    // std::vector<std::shared_ptr<WaveDeformer>> m_waves;
    // std::shared_ptr<TurbulenceDeformer> m_surfaceTurbulence;

    ObjectInspectorPtr m_objectInspector;

    bool m_dirty{true}; // Flag to track if properties have changed

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        // Create and add axis for orientation

        auto axis = Axis::create();
        this->add(axis);

        // Create some materials with different colors

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

        // Set mesh usage for dynamic drawing

        mmDefaultMeshUsage(GL_DYNAMIC_DRAW);

        // Create a deformable sphere and set its properties

        m_defSphere = DeformablePrimitive<Sphere>::create();
        m_defSphere->primitive()->set(3.0);
        m_defSphere->refresh(); // Update geometry and deformer setup
        m_defSphere->setMaterial(whiteMaterial);
        m_defSphere->setWireframe(false);
        m_defSphere->setPos(glm::vec3(0, 0, 0));

        // 1. Apply uniform random jitter to the sphere

        m_uniformNoise =
            RandomDeformer::create(RandomDeformer::NoiseType::UNIFORM, RandomDeformer::DeformationMode::ADDITIVE);
        m_uniformNoise->setIntensity(0.1f);
        m_uniformNoise->setSeed(42);

        m_defSphere->addDeformer(m_uniformNoise);
        m_defSphere->applyDeformers();

        std::cout << "Applied uniform random jitter to sphere" << std::endl;

        // 2. Apply Gaussian noise to a plane

        m_defPlane = DeformablePrimitive<Plane>::create(4.0f, 4.0f, 100, 100);

        m_gaussianNoise =
            RandomDeformer::create(RandomDeformer::NoiseType::GAUSSIAN, RandomDeformer::DeformationMode::DIRECTIONAL);
        m_gaussianNoise->setIntensity(0.1f);
        m_gaussianNoise->setDirection(glm::vec3(0, 1, 0)); // Vertical displacement
        m_gaussianNoise->setSeed(123);

        m_defPlane->addDeformer(m_gaussianNoise);
        m_defPlane->setMaterial(greenMaterial);
        m_defPlane->setPos(glm::vec3(-10, 0, 0)); // Position left of the sphere
        m_defPlane->applyDeformers();

        std::cout << "Applied Gaussian noise to plane" << std::endl;

        // 3. Apply Perlin noise for organic deformation to a cylinder

        m_defCylinder = DeformablePrimitive<CappedCylinder>::create(1.0f, 3.0f, 24);

        m_perlinNoise =
            RandomDeformer::create(RandomDeformer::NoiseType::PERLIN, RandomDeformer::DeformationMode::RADIAL);
        m_perlinNoise->setIntensity(0.1f);
        m_perlinNoise->setFrequency(2.0f);
        m_perlinNoise->setCenter(glm::vec3(0, 0, 0));

        m_defCylinder->addDeformer(m_perlinNoise);
        m_defCylinder->setMaterial(redMaterial);
        m_defCylinder->setPos(glm::vec3(10, 0, 0)); // Position to the right of the sphere
        m_defCylinder->applyDeformers();

        // Add all primitives to the scene

        this->add(m_defSphere);
        this->add(m_defPlane);
        this->add(m_defCylinder);

        // Create an object inspector UI for the sphere

        m_objectInspector = ObjectInspector::create("Inspector");
        m_objectInspector->setObject(m_defSphere);
        m_objectInspector->setPropertyChangedCallback(
            std::bind(&ExampleWindow::onPropertiesChanged, this, std::placeholders::_1));

        this->addUiWindow(m_objectInspector);

        // Set initial camera position

        this->cameraManipulator()->setCameraPosition(glm::vec3(0, 5, 20));

        return 0;
    }

    void onPropertiesChanged(const std::string &name)
    {
        // Handle property changes if needed

        m_dirty = true;
    }

    virtual void onUpdate()
    {
        // Update the object inspector with the current object

        float time = this->elapsedTime();

        m_defCylinder->applyDeformers();
        m_defPlane->applyDeformers();
        m_defSphere->applyDeformers();
    }

    virtual void onKey(int key, int scancode, int action, int mods) override
    {
        if (key == GLFW_KEY_1)
        {
            m_objectInspector->setObject(m_defSphere);
        }
        else if (key == GLFW_KEY_2)
        {
            m_objectInspector->setObject(m_defPlane);
        }
        else if (key == GLFW_KEY_3)
        {
            m_objectInspector->setObject(m_defCylinder);
        }

        if ((key == GLFW_KEY_1) && mods == GLFW_MOD_SHIFT)
        {
            m_objectInspector->setObject(m_uniformNoise);
        }
        else if ((key == GLFW_KEY_2) && mods == GLFW_MOD_SHIFT)
        {
            m_objectInspector->setObject(m_gaussianNoise);
        }
        else if ((key == GLFW_KEY_3) && mods == GLFW_MOD_SHIFT)
        {
            m_objectInspector->setObject(m_perlinNoise);
        }

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

    auto window = ExampleWindow::create(1280, 800, "Deformer");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
