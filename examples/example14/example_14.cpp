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
        // this->setSelectionEnabled(true);
        // this->setRenderToTexture(true);

        auto axis = Axis::create();
        auto grid = Grid::create();
        grid->setType(GridType::Markers);

        auto box = Box::create();
        box->setSize(0.5, 0.5, 0.5);
        box->refresh();

        auto model = ModelLoader::loadModel("assets/city.ac");

        auto material = Material::create();
        material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        material->setDiffuseColor(glm::vec4(0.8, 0.8, 0.0, 1.0));
        material->setAmbientColor(glm::vec4(0.2, 0.2, 0.0, 1.0));
        model->setMaterial(material);

        box->setMaterial(material);

        this->add(axis);
        this->add(grid);
        this->add(model);
        this->add(box);

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

    auto window = ExampleWindow::create(800, 800, "Example 6");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
