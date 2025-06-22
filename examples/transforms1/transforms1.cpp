/**
 * @file transforms1.cpp
 * @brief Texture example
 * @author Jonas Lindemann
 * @example transforms1.cpp
 * @ingroup general_examples
 *
 * This example demonstrates the use of various transformations
 * (translation, rotation, scaling) on a set of arrow objects
 * in a 3D scene using the IVF library.
 * It creates a grid of arrows with random orientations
 * and positions, showcasing the use of the TransformNode class.
 */

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
        // Create and add an axis to the scene for reference.

        AxisPtr axis = Axis::create();

        this->add(axis);

        // Create a yellow material for potential use.

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        // Create a red material for the arrows.

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        // Create an arrow object and set its properties.

        auto arrow = Arrow::create();
        arrow->setArrowType(ArrowType::RightSided);
        arrow->setLength(0.5);
        arrow->setBodyRadius(0.05);
        arrow->setConeRadius(0.1);
        arrow->setMaterial(redMat);

        // Create a 5x5x5 grid of arrows, each with a random orientation and position.

        for (auto i = 0; i < 5; i++)
            for (auto j = 0; j < 5; j++)
                for (auto k = 0; k < 5; k++)
                {
                    // Create a transform node for each arrow.

                    auto xfm = Transform::create();
                    xfm->add(arrow);
                    xfm->setPos(glm::vec3(-2.0 + i, -2.0 + j, -2.0 + k));
                    xfm->rotateToVector(glm::vec3(random(-1.0, 1.0), random(-1.0, 1.0), random(-1.0, 1.0)));
                    this->add(xfm);
                }

        // Return 0 to indicate successful setup.

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

    auto window = ExampleWindow::create(1280, 800, "Transforms");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
