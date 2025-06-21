/**
 * @file animation3.cpp
 * @brief Simple animation example
 * @author Jonas Lindemann
 * @example animation3.cpp
 * @ingroup animation_examples
 * 
 * This example demonstrates advanced animation techniques. It includes spline and keyframe animations with a sphere moving along a spline path.
 *
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

#include <ivfmath/spline.h>

using namespace ivf;
using namespace ivfui;
using namespace ivfmath;

using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    CubePtr m_cube;
    SpherePtr m_sphere;
    LineTracePtr m_trace1;
    LineTracePtr m_trace2;
    MaterialPtr m_yellowMat;
    MaterialPtr m_selectionMaterial;
    SplineAnimationPtr m_splineAnim;
    KeyframeAnimationPtr m_keyframeAnim;
    KeyframeInterpolatorPtr m_keyframeInterpolator;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        // Optionally enable selection and render-to-texture features

        // this->setSelectionEnabled(true);
        // this->setRenderToTexture(true);

        // Create and add axis and grid to the scene

        auto axis = Axis::create();
        auto grid = Grid::create();
        grid->setType(GridType::Markers);

        this->add(axis);
        this->add(grid);

        // Create a yellow material for later use

        m_yellowMat = Material::create();
        m_yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        // Create a red material for the sphere

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        // Create a cube, set its size and material

        m_cube = Cube::create();
        m_cube->setSize(0.5);
        m_cube->setMaterial(m_yellowMat);

        // Create a sphere, set its material and radius
        
        m_sphere = Sphere::create();
        m_sphere->setMaterial(redMat);
        m_sphere->setRadius(0.1);
        m_sphere->refresh();

        // Create a spline animation object

        m_splineAnim = SplineAnimation::create();

        // Define a spline path with four control points

        auto spline = Spline::create();
        spline->addPoint(glm::vec3(0.0f, 0.0f, 0.0f));
        spline->addPoint(glm::vec3(1.0f, 2.0f, 1.0f));
        spline->addPoint(glm::vec3(2.0f, 0.0f, 0.0f));
        spline->addPoint(glm::vec3(3.0f, 1.0f, -1.0f));

        // Visualize the spline control points with small yellow spheres

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

        // Configure the spline animation for the sphere

        m_splineAnim->setSpline(spline);
        m_splineAnim->setAnimatedNode(m_sphere);
        m_splineAnim->setInterpolationMode(SplineInterpolationMode::Distance);
        m_splineAnim->setSpeed(1.0);
        m_splineAnim->setAnimMode(SplineAnimMode::PingPong);

        // Create a keyframe interpolator and add keyframes for the cube animation

        m_keyframeInterpolator = KeyframeInterpolator::create();
        m_keyframeInterpolator->addKeyframe(
            KeyframeData(0.0, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0)));
        m_keyframeInterpolator->addKeyframe(
            KeyframeData(5.0, glm::vec3(0.0, 2.0, 0.0), glm::vec3(45.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0)));
        m_keyframeInterpolator->addKeyframe(
            KeyframeData(10.0, glm::vec3(-3.0, 2.0, 0.0), glm::vec3(0.0, 45.0, 0.0), glm::vec3(1.0, 1.0, 1.0)));
        m_keyframeInterpolator->addKeyframe(
            KeyframeData(15.0, glm::vec3(-3.0, 0.0, -3.0), glm::vec3(0.0, 0.0, 45.0), glm::vec3(1.0, 1.0, 1.0)));
        m_keyframeInterpolator->setInterpolationType(InterpolationType::CatmullRom);

        // Create a keyframe animation for the cube and start playing it

        m_keyframeAnim = KeyframeAnimation::create(m_cube);
        m_keyframeAnim->setInterpolator(m_keyframeInterpolator);
        m_keyframeAnim->play();

        // Create line traces to visualize the animation paths of the sphere and cube

        m_trace1 = LineTrace::create(300);
        m_trace2 = LineTrace::create(300);

        // Add all objects to the scene
        this->add(m_sphere);
        this->add(p1);
        this->add(p2);
        this->add(p3);
        this->add(p4);
        this->add(m_cube);
        this->add(m_trace1);
        this->add(m_trace2);

        return 0;
    }

    virtual void onUpdate()
    {
        // Update the spline animation for the sphere (advance by 1/60 second)

        m_splineAnim->update(1.0 / 60.0);

        // Update the keyframe animation for the cube (advance by 1/60 second)
        
        m_keyframeAnim->update(1.0 / 60.0);

        // Add the current position of the sphere to its trace and refresh the trace

        m_trace1->add(m_sphere->pos());
        m_trace1->refresh();

        // Add the current position of the cube to its trace and refresh the trace

        m_trace2->add(m_cube->pos());
        m_trace2->refresh();
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
