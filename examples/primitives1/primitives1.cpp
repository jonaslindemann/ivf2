/**
 * @file primitives1.cpp
 * @brief Primitives example
 * @author Jonas Lindemann
 * @example primitives1.cpp
 * @ingroup primivites_examples
 *
 * This example demonstrates the creation and rendering of various 3D geometric primitives
 * including spheres, boxes, cylinders, cones, and more. It sets up a basic scene with an axis,
 * grid, and multiple primitive shapes positioned in 3D space.
 * The scene allows for camera manipulation to view the primitives from different angles.
 */

#include <iostream>
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
public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Create an axis for orientation

        auto axis = Axis::create();

        // Create a grid for the scene

        auto grid = Grid::create();
        grid->setTicks(15, 15, 15); // Set grid ticks
        grid->setType(GridType::LinesAndMarkers); // Set grid type

        // Create a material and configure its properties

        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Set diffuse color
        material->setUseTexture(true); // Enable texture (overwritten below)
        material->setUseTexture(false); // Disable texture
        material->setShininess(50.0); // Set shininess

        // Create a sphere primitive and set its properties

        auto sphere = Sphere::create();
        sphere->setMaterial(material);
        sphere->setPos(glm::vec3(-3.0, 0.0, -3.0));

        // Create a box primitive and set its properties

        auto box = Box::create();
        box->setMaterial(material);
        box->setPos(glm::vec3(0.0, .0, -3.0));

        // Create a rounded box primitive and set its properties

        auto rbox = RoundedBox::create();
        rbox->setMaterial(material);
        rbox->setPos(glm::vec3(3.0, 0.0, -3.0));

        // Create a capped cylinder primitive and set its properties

        auto capCyl = CappedCylinder::create();
        capCyl->setMaterial(material);
        capCyl->setPos(glm::vec3(-3.0, 0.0, 0.0));

        // Create an open cylinder primitive and set its properties

        auto opCyl = Cylinder::create();
        opCyl->setMaterial(material);
        opCyl->setPos(glm::vec3(0.0, 0.0, 0.0));

        // Create a cone primitive and set its properties

        auto cone = Cone::create();
        cone->setMaterial(material);
        cone->setPos(glm::vec3(3.0, 0.0, 0.0));

        // Create a capped cone primitive and set its properties

        auto capCone = CappedCone::create();
        capCone->setMaterial(material);
        capCone->setPos(glm::vec3(-3.0, 0.0, 3.0));

        // Create a dodecahedron primitive and set its properties

        auto dodeka = Dodecahedron::create();
        dodeka->setMaterial(material);
        dodeka->setPos(glm::vec3(0.0, 0.0, 3.0));

        // Create a capsule primitive and set its properties

        auto capsule = Capsule::create();
        capsule->setMaterial(material);
        capsule->setPos(glm::vec3(3.0, 0.0, 3.0));

        // Create a capped tube primitive and set its properties

        auto cappedTube = CappedTube::create();
        cappedTube->setMaterial(material);
        cappedTube->setPos(glm::vec3(-3.0, 0.0, 6.0));

        // Create a tube primitive and set its properties

        auto tube = Tube::create();
        tube->setMaterial(material);
        tube->setPos(glm::vec3(0.0, 0.0, 6.0));

        // Create a disk primitive and set its properties

        auto disk = Disk::create();
        disk->setMaterial(material);
        disk->setPos(glm::vec3(3.0, 0.0, 6.0));

        // Create a plane primitive and set its properties

        auto plane = Plane::create();
        plane->setMaterial(material);
        plane->setPos(glm::vec3(-3.0, 0.0, 9.0));

        // Add primitives to the scene

        this->add(box);
        this->add(rbox);
        this->add(sphere);
        this->add(capCyl);
        this->add(opCyl);
        this->add(cone);
        this->add(capCone);
        this->add(dodeka);
        this->add(capsule);
        this->add(cappedTube);
        this->add(tube);
        this->add(disk);
        this->add(plane);

        // Add axis and grid to the scene

        this->add(axis);
        this->add(grid);

        // Set the camera position for the scene

        this->cameraManipulator()->setCameraPosition(glm::vec3(15.0, 5.0, 0.0));

        return 0;
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

    auto window = ExampleWindow::create(800, 800, "Primitives");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
