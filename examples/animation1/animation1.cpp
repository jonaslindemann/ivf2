#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    CubePtr m_cube;
    SpherePtr m_sphere;
    TransformPtr m_sphereXfm;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        AxisPtr axis = Axis::create();

        this->add(axis);

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        auto sph1 = Sphere::create();
        sph1->setMaterial(yellowMat);
        sph1->setRadius(0.5);
        sph1->refresh();
        sph1->setPos(glm::vec3(0.0, 0.0, 0.0));

        m_cube = Cube::create();
        m_cube->setMaterial(redMat);

        m_sphere = Sphere::create();
        m_sphere->setMaterial(yellowMat);
        m_sphere->setRadius(0.25);
        m_sphere->refresh();
        m_sphere->setPos(glm::vec3(2.0, 0.0, 0.0));

        m_sphereXfm = Transform::create();
        m_sphereXfm->add(m_sphere);

        this->add(m_cube);
        this->add(m_sphereXfm);

        return 0;
    }

    virtual void onUpdate()
    {
        m_cube->setRotAxis(glm::vec3(1.0, 1.0, 1.0));
        m_cube->setRotAngle(20.0 * elapsedTime());

        m_sphereXfm->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
        m_sphereXfm->setRotAngle(20.0 * elapsedTime());
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
