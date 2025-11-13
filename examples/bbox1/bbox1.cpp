/**
 * @file scene_ui1.cpp
 * @brief Hello IVF example
 * @author Jonas Lindemann
 * @example scene_ui1.cpp
 * @ingroup ui_examples
 *
 * This example demonstrates the use of the IVF library to create a scene with
 * various geometric shapes, a main menu, and multiple UI windows.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/extents.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Randomly seed
        
        randomSeed();
        
        // Show axis

        this->enableAxis();

        // Create a container node to test bbox calculation

        auto container = Transform::create();

        // Add 50 random geometric shapes to the scene

        for (auto i = 0; i < 50; i++)
        {
            // Generate a random number to select shape type

            double r = random(0.0, 9.0);

            TransformNodePtr node;

            // Create a random shape based on r

            if (r < 1.0)
                node = Cube::create();
            else if ((r >= 1) && (r < 2.0))
                node = Sphere::create(random(0.2, 1.0));
            else if ((r >= 2.0) && (r < 3.0))
                node = Box::create(glm::vec3(random(0.2, 2.0), random(0.2, 2.0), random(0.2, 2.0)));
            else if ((r >= 3.0) && (r < 4.0))
                node = RoundedBox::create(glm::vec3(random(0.2, 2.0), random(0.2, 2.0), random(0.2, 2.0)));
            else if ((r >= 4.0) && (r < 5.0))
                node = CappedCylinder::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 5.0) && (r < 6.0))
                node = Cylinder::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 6.0) && (r < 7.0))
                node = CappedCone::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 7.0) && (r < 8.0))
                node = Cone::create(random(0.2, 1.0), random(0.2, 1.0));
            else
                node = Dodecahedron::create(random(0.2, 1.0));

            // Create and set a random material for the shape

            auto material = Material::create();
            material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));
            material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

            // Set a random position for the shape

            node->setPos(glm::vec3(random(-5.0, 5.0), random(-5.0, 5.0), random(-5.0, 5.0)));
            node->setMaterial(material);

            // Add the shape to the scene

            container->add(node);
        }
        
        container->setPos(glm::vec3(0.0, 10.0, 0.0));

        this->add(container);

        auto bbox_scene = this->scene()->worldBoundingBox();
        auto bbox_container = container->worldBoundingBox();
        auto bbox_container_local = container->localBoundingBox();

        auto extents_scene = Extents::create(bbox_scene);
        this->add(extents_scene);

        //this->scaleAllSizes(4.0);

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0, 3.0, 20.0));

        std::printf("Scene world bounding box min: (%f, %f, %f)\n", bbox_scene.min().x, bbox_scene.min().y,
                    bbox_scene.min().z);
        std::printf("Scene world bounding box max: (%f, %f, %f)\n", bbox_scene.max().x, bbox_scene.max().y,
                    bbox_scene.max().z);
        std::printf("Container world bounding box min: (%f, %f, %f)\n", bbox_container.min().x, bbox_container.min().y,
                    bbox_container.min().z);
        std::printf("Container world bounding box max: (%f, %f, %f)\n", bbox_container.max().x, bbox_container.max().y,
                    bbox_container.max().z);
        std::printf("Container local bounding box min: (%f, %f, %f)\n", bbox_container_local.min().x, bbox_container_local.min().y,
                    bbox_container_local.min().z);
        std::printf("Container local bounding box max: (%f, %f, %f)\n", bbox_container_local.max().x, bbox_container_local.max().y,
                    bbox_container_local.max().z);

        return 0;
    }

    void onKey(int key, int scancode, int action, int mods)
    {
        // Handle key events

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
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

    auto window = ExampleWindow::create(1280, 800, "Example 2");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
