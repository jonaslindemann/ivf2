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
    ArrowPtr m_arrow;

    double m_t = 0.0;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        AxisPtr axis = Axis::create();

        this->add(axis);

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        auto arrow = Arrow::create();
        arrow->setArrowType(ArrowType::RightSided);
        arrow->setLength(0.5);
        arrow->setBodyRadius(0.05);
        arrow->setConeRadius(0.1);
        arrow->setMaterial(redMat);

        for (auto i = 0; i < 5; i++)
            for (auto j = 0; j < 5; j++)
                for (auto k = 0; k < 5; k++)
                {
                    auto xfm = Transform::create();
                    xfm->add(arrow);
                    xfm->setPos(glm::vec3(-2.0 + i, -2.0 + j, -2.0 + k));
                    xfm->rotateToVector(glm::vec3(random(-1.0, 1.0), random(-1.0, 1.0), random(-1.0, 1.0)));
                    this->add(xfm);
                }

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

    auto window = ExampleWindow::create(800, 800, "Transforms");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
