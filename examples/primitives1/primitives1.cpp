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
        auto axis = Axis::create();
        auto grid = Grid::create();
        grid->setTicks(15, 15, 15);
        grid->setType(GridType::LinesAndMarkers);

        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        material->setUseTexture(true);
        material->setUseTexture(false);
        material->setShininess(50.0);

        auto sphere = Sphere::create();
        sphere->setMaterial(material);
        sphere->setPos(glm::vec3(-3.0, 0.0, -3.0));

        auto box = Box::create();
        box->setMaterial(material);
        box->setPos(glm::vec3(0.0, .0, -3.0));

        auto rbox = RoundedBox::create();
        rbox->setMaterial(material);
        rbox->setPos(glm::vec3(3.0, 0.0, -3.0));

        auto capCyl = CappedCylinder::create();
        capCyl->setMaterial(material);
        capCyl->setPos(glm::vec3(-3.0, 0.0, 0.0));

        auto opCyl = Cylinder::create();
        opCyl->setMaterial(material);
        opCyl->setPos(glm::vec3(0.0, 0.0, 0.0));

        auto cone = Cone::create();
        cone->setMaterial(material);
        cone->setPos(glm::vec3(3.0, 0.0, 0.0));

        auto capCone = CappedCone::create();
        capCone->setMaterial(material);
        capCone->setPos(glm::vec3(-3.0, 0.0, 3.0));

        auto dodeka = Dodecahedron::create();
        dodeka->setMaterial(material);
        dodeka->setPos(glm::vec3(0.0, 0.0, 3.0));

        auto capsule = Capsule::create();
        capsule->setMaterial(material);
        capsule->setPos(glm::vec3(3.0, 0.0, 3.0));

        auto cappedTube = CappedTube::create();
        cappedTube->setMaterial(material);
        cappedTube->setPos(glm::vec3(-3.0, 0.0, 6.0));

        auto tube = Tube::create();
        tube->setMaterial(material);
        tube->setPos(glm::vec3(0.0, 0.0, 6.0));

        auto disk = Disk::create();
        disk->setMaterial(material);
        disk->setPos(glm::vec3(3.0, 0.0, 6.0));

        auto plane = Plane::create();
        plane->setMaterial(material);
        plane->setPos(glm::vec3(-3.0, 0.0, 9.0));

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

        this->add(axis);
        this->add(grid);

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
