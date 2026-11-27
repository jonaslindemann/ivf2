// A small scene window built against the installed ivf2, for the web package test: CI builds
// it as a web page with ivf2_web_executable() and opens it in headless Chrome. The same code
// runs on the desktop. The Web guide (docs/docs/getting_started/web.md) shows this program.

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/glfw_application.h>
#include <ivfui/glfw_scene_window.h>

class WebAppWindow : public ivfui::GLFWSceneWindow {
public:
    using GLFWSceneWindow::GLFWSceneWindow;

    int onSetup() override
    {
        auto material = ivf::Material::create();
        material->setDiffuseColor(glm::vec4(0.9f, 0.6f, 0.2f, 1.0f));

        auto sphere = ivf::Sphere::create(0.8);
        sphere->setMaterial(material);
        sphere->setPos(glm::vec3(-1.2f, 0.0f, 0.0f));
        add(sphere);

        auto cube = ivf::Cube::create();
        cube->setMaterial(material);
        cube->setPos(glm::vec3(1.2f, 0.0f, 0.0f));
        add(cube);

        enableAxis();
        cameraManipulator()->setCameraPosition(glm::vec3(0.0f, 2.0f, 6.0f));
        return 0;
    }
};

int main()
{
    auto app = ivfui::GLFWApplication::create();
    auto window = std::make_shared<WebAppWindow>(1280, 720, "ivf2 in the browser");
    app->addWindow(window);
    return app->loop();
}
