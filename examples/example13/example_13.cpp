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

#include <ivfmath/spline.h>

using namespace ivf;
using namespace ivfui;
using namespace ivfmath;

using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    CubePtr m_cube;
    SpherePtr m_sphere;
    LineTracePtr m_trace;
    MaterialPtr m_yellowMat;
    MaterialPtr m_selectionMaterial;
    SplineAnimationPtr m_splineAnim;

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

        this->add(axis);
        this->add(grid);

        m_yellowMat = Material::create();
        m_yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        m_cube = Cube::create();
        m_cube->setMaterial(m_yellowMat);

        m_sphere = Sphere::create();
        m_sphere->setMaterial(redMat);
        m_sphere->setRadius(0.1);
        m_sphere->refresh();

        m_splineAnim = SplineAnimation::create();

        auto spline = Spline::create();
        spline->addPoint(glm::vec3(0.0f, 0.0f, 0.0f));
        spline->addPoint(glm::vec3(1.0f, 2.0f, 1.0f));
        spline->addPoint(glm::vec3(2.0f, 0.0f, 0.0f));
        spline->addPoint(glm::vec3(3.0f, 1.0f, -1.0f));

        auto p1 = Sphere::create(0.05);
        p1->setMaterial(m_yellowMat);
        p1->setPos(glm::vec3(0.0f, 0.0f, 0.0f));

        auto p2 = Sphere::create(0.05);
        p2->setMaterial(m_yellowMat);
        p2->setPos(glm::vec3(1.0f, 2.0f, 1.0f));

        auto p3 = Sphere::create(0.05);
        p3->setMaterial(m_yellowMat);
        p3->setPos(glm::vec3(2.0f, 0.0f, 0.0f));

        auto p4 = Sphere::create(0.05);
        p4->setMaterial(m_yellowMat);
        p4->setPos(glm::vec3(3.0f, 1.0f, -1.0f));

        m_splineAnim->setSpline(spline);
        m_splineAnim->setAnimatedNode(m_sphere);
        m_splineAnim->setInterpolationMode(SplineInterpolationMode::Distance);
        m_splineAnim->setSpeed(1.0);
        m_splineAnim->setAnimMode(SplineAnimMode::PingPong);

        m_trace = LineTrace::create(300);

        this->add(m_sphere);
        this->add(p1);
        this->add(p2);
        this->add(p3);
        this->add(p4);
        this->add(m_trace);

        return 0;
    }

    virtual void onUpdate()
    {
        m_splineAnim->update(1.0 / 60.0);

        m_trace->add(m_sphere->pos());
        m_trace->refresh();
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
