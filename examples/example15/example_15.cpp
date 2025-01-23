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
#include <ivf/node_visitor.h>

#include <ivfmath/spline.h>

#include "grid_layout.h"

using namespace ivf;
using namespace ivfui;
using namespace ivfmath;

using namespace std;

glm::vec3 repeller(float x, float y, float z)
{
    float r = sqrt(x * x + y * y + z * z);
    return glm::vec3(-x / (r * r), -y / (r * r), -z / (r * r));
}

class FunctionVisitor : public NodeVisitor {
private:
public:
    virtual void visit(Node *node) override
    {
        auto xfm = dynamic_cast<Transform *>(node);
        if (xfm == nullptr)
            return;
        auto pos = xfm->pos();
        auto rep = repeller(pos.x, pos.y, pos.z);
        xfm->setPos(pos - rep * 0.05f);
    }
};

class ExampleWindow : public GLFWSceneWindow {
private:
    FunctionVisitor m_visitor;
    CompositeNodePtr m_nodes;

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

        this->enableHeadlight();

        auto axis = Axis::create();
        auto grid = Grid::create();
        grid->setType(GridType::Markers);

        auto material = Material::create();
        material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        material->setDiffuseColor(glm::vec4(0.8, 0.8, 0.0, 1.0));
        material->setAmbientColor(glm::vec4(0.2, 0.2, 0.0, 1.0));

        GridLayout layout(10, 10, 10, 1.0, 1.0, 1.0);

        auto sphere = Sphere::create(0.1);
        sphere->setMaterial(material);

        m_nodes = CompositeNode::create();

        for (auto i = 0; i < layout.size(); i++)
        {
            auto xfm = Transform::create();
            xfm->add(sphere);
            m_nodes->add(xfm);
        }

        layout.apply(m_nodes);

        this->add(axis);
        this->add(grid);
        this->add(m_nodes);

        return 0;
    }

    virtual void onUpdate()
    {
        m_nodes->accept(&m_visitor);
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

    auto window = ExampleWindow::create(800, 800, "Example 6");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
