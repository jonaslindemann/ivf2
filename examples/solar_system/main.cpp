
#include "solar_window.h"

using namespace solar_ui;

int main( )
{
    auto app = ivfui::GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = SolarWindow::create(800, 800, "Example 6");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
