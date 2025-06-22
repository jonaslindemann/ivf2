/**
 * @file assimp1.cpp
 * @brief Assimp model loading example
 * @author Jonas Lindemann
 * @example assimp1.cpp
 * @ingroup file_io_examples
 *
 * This example demonstrates loading and displaying a 3D model using the Assimp library.
 * It creates a scene with a grid, axis, and a model loaded from an .ac file.
 */

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <ivf/node_visitor.h>
#include <ivf/stock_shaders.h>
#include <ivf/blur_effect.h>
#include <ivf/spline_animation.h>
#include <ivf/keyframe_animation.h>
#include <ivf/model_loader.h>

#include <ivfmath/spline.h>

using namespace ivf;
using namespace ivfui;
using namespace ivfmath;

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

    virtual int onSetup() override
    {
        // Enable a headlight for basic scene illumination

        this->enableHeadlight();

        // Create and add an axis to the scene for orientation reference

        auto axis = Axis::create();

        // Create and add a grid to the scene, using marker style

        auto grid = Grid::create();
        grid->setType(GridType::Markers);

        // Create a box node, set its size, and refresh its geometry

        auto box = Box::create();
        box->setSize(0.5, 0.5, 0.5);
        box->refresh();

        // Load a 3D model from an .ac file using Assimp

        auto model = ModelLoader::loadModel("assets/city.ac");

        // Create a material and set its properties

        auto material = Material::create();
        material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        material->setDiffuseColor(glm::vec4(0.8, 0.8, 0.0, 1.0));
        material->setAmbientColor(glm::vec4(0.2, 0.2, 0.0, 1.0));

        // Assign the material to the loaded model

        model->setMaterial(material);

        // Assign the same material to the box

        box->setMaterial(material);

        // Add all nodes to the scene

        this->add(model);

        // Return 0 to indicate successful setup

        return 0;
    }

    virtual void onUpdate()
    {}
};

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = ExampleWindow::create(1280, 800, "Assimp");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
