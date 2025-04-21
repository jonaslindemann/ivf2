#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include "lissajou_window.h"

using namespace ivf;
using namespace ivfui;
using namespace std;

class Lissajous {
private:
    float m_a, m_b, m_c, m_d, m_e, m_f, m_g, m_h, m_i;

public:
    Lissajous(float a = 1.0, float b = 1.0, float c = 1.0, float d = 0.0, float e = 1.0, float f = 0.0, float g = 1.0,
              float h = 1.0, float i = 0.0)
        : m_a(a), m_b(b), m_c(c), m_d(d), m_e(e), m_f(f), m_g(g), m_h(h), m_i(i)
    {}

    void setParameters(float a = 1.0, float b = 1.0, float c = 1.0, float d = 0.0, float e = 1.0, float f = 0.0,
                       float g = 1.0, float h = 1.0, float i = 0.0)
    {
        m_a = a;
        m_b = b;
        m_c = c;
        m_d = d;
        m_e = e;
        m_f = f;
        m_g = g;
        m_h = h;
        m_i = i;
    }

    glm::vec3 operator()(float t)
    {
        return glm::vec3(m_a * sin(m_b * t + m_d), m_c * sin(m_e * t + m_f), m_g * sin(m_h * t + m_i));
    }
};

class ExampleWindow : public GLFWSceneWindow {
private:
    SpherePtr m_sphere;
    LineTracePtr m_trace;

    Lissajous m_lissajous;
    LissajouWindowPtr m_lissajouWindow;

    float m_speed{1.0};

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title), m_speed(1.0)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        this->setRenderToTexture(true);

        AxisPtr axis = Axis::create();

        this->add(axis);

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        m_sphere = Sphere::create();
        m_sphere->setMaterial(yellowMat);
        m_sphere->setRadius(0.1);
        m_sphere->refresh();
        m_sphere->setPos(glm::vec3(0.0, 0.0, 0.0));

        this->add(m_sphere);

        m_trace = LineTrace::create(300);

        // (m_a * sin(m_b * t + m_d), m_c * sin(m_e * t + m_f), m_g * sin(m_h * t + m_i)

        m_lissajous.setParameters(1.0, 1.0, 1.0, 0.7, 3.0, 0.4, 1.0, 2.0, 1.0);
        //                        m_a, m_b, m_c, m_d, m_e, m_f, m_g, m_h, m_i

        this->add(m_trace);

        m_lissajouWindow = LissajouWindow::create();
        this->addUiWindow(m_lissajouWindow);

        return 0;
    }

    virtual void onUpdate() override
    {
        if (m_lissajouWindow->is_dirty())
        {
            float a, b, c, d, e, f, g, h, i;
            m_lissajouWindow->get_params(a, b, c, d, e, f, g, h, i);
            m_lissajous.setParameters(a, b, c, d, e, f, g, h, i);
            m_speed = m_lissajouWindow->speed();
            m_trace->setSize(m_lissajouWindow->size());
            m_trace->reset();
        }

        auto pos = m_lissajous(elapsedTime() * m_speed);
        m_sphere->setPos(pos);
        m_trace->add(pos);
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

    auto window = ExampleWindow::create(800, 800, "Animation");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
