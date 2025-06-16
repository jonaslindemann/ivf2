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
    PlanePtr m_plane;
    std::shared_ptr<DeformablePrimitive<Sphere>> m_deformable;

    std::vector<std::shared_ptr<WaveDeformer>> m_waves;
    std::shared_ptr<TurbulenceDeformer> m_surfaceTurbulence;

    ObjectInspectorPtr m_objectInspector;

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

        auto sphere = DeformablePrimitive<Sphere>::create();
        sphere->primitive()->set(3.0);
        sphere->refresh(); // Update geometry and deformer setup
        sphere->setMaterial(whiteMaterial);
        sphere->setWireframe(false);
        sphere->setPos(glm::vec3(0, 0, 0));

        // 1. Uniform random jitter
        auto uniformNoise =
            RandomDeformer::create(RandomDeformer::NoiseType::UNIFORM, RandomDeformer::DeformationMode::ADDITIVE);
        uniformNoise->setIntensity(0.1f);
        uniformNoise->setSeed(42);

        sphere->addDeformer(uniformNoise);
        sphere->applyDeformers();

        std::cout << "Applied uniform random jitter to sphere" << std::endl;

        // 2. Gaussian noise on a plane
        auto plane = DeformablePrimitive<Plane>::create(4.0f, 4.0f, 100, 100);

        auto gaussianNoise =
            RandomDeformer::create(RandomDeformer::NoiseType::GAUSSIAN, RandomDeformer::DeformationMode::DIRECTIONAL);
        gaussianNoise->setIntensity(0.1f);
        gaussianNoise->setDirection(glm::vec3(0, 1, 0)); // Vertical displacement
        gaussianNoise->setSeed(123);

        plane->addDeformer(gaussianNoise);
        plane->setMaterial(greenMaterial);
        plane->setPos(glm::vec3(-10, 0, 0)); // Position left the sphere
        plane->applyDeformers();

        std::cout << "Applied Gaussian noise to plane" << std::endl;

        // 3. Perlin noise for organic deformation
        auto cylinder = DeformablePrimitive<CappedCylinder>::create(1.0f, 3.0f, 24);

        auto perlinNoise =
            RandomDeformer::create(RandomDeformer::NoiseType::PERLIN, RandomDeformer::DeformationMode::RADIAL);
        perlinNoise->setIntensity(0.1f);
        perlinNoise->setFrequency(2.0f);
        perlinNoise->setCenter(glm::vec3(0, 0, 0));

        cylinder->addDeformer(perlinNoise);
        cylinder->setMaterial(redMaterial);
        cylinder->setPos(glm::vec3(10, 0, 0)); // Position to the right of the sphere
        cylinder->applyDeformers();

        this->add(sphere);
        this->add(plane);
        this->add(cylinder);

        m_objectInspector = ObjectInspector::create("Sphere");
        m_objectInspector->setObject(sphere);

        this->addUiWindow(m_objectInspector);

        this->cameraManipulator()->setCameraPosition(glm::vec3(0, 5, 20));

        return 0;
    }

    virtual void onUpdate()
    {
        float time = this->elapsedTime();
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
