# The Scene Window

To make it easier to create 3D applications ivf2 provides a specialised version of the `GLFWWindow` class called `GLFWSceneWindow`. This class manages the scene graph and provides a number of convenience functions for rendering and updating the scene. The `GLFWSceneWindow` class is a subclass of `GLFWWindow` and inherits all of its functionality. 

Instead of maintaining your own scene instance nodes can be added in the `onSetup()` method using the `add(...)` method. There is also no need to implement the `onRender()` method as this is handled by the `GLFWSceneWindow` class. The `onUpdate()` method is still available for you to implement your own logic.

The class also supports rendering to a texture. This is useful for creating effects such as shadow mapping or post-processing, which is also supported by the class. 

The class also automates the creation of UiWindow derived ImGui windows. 

## Creating a Scene Window

To create a scene window you need to create a subclass of `GLFWSceneWindow` and implement the `onSetup()` and `onUpdate()` methods. The `onSetup()` method is called once when the window is created and is used to set up the scene. The `onUpdate()` method is called every frame and is used to update the scene.

The example below shows how to create a simple scene window with a few shapes and a light source. 

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
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

    int onSetup()
    {
        auto lightMgr = LightManager::create();

        auto pointLight1 = lightMgr->addPointLight();
        pointLight1->setEnabled(true);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.0, 0.0);
        pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));
        lightMgr->apply();

        auto axis = Axis::create();

        this->add(axis);

        for (auto i = 0; i < 500; i++)
        {
            double r = random(0.0, 9.0);

            TransformNodePtr node;

            if (r < 1.0)
                node = Cube::create();
            else if ((r >= 1) && (r < 2.0))
                node = Sphere::create(random(0.2, 1.0));
            else if ((r >= 2.0) && (r < 3.0))
                node = Box::create(glm::vec3(random(0.2, 2.0), random(0.2, 2.0), random(0.2, 2.0)));
            else if ((r >= 3.0) && (r < 4.0))
                node = RoundedBox::create(glm::vec3(random(0.2, 2.0), random(0.2, 2.0), random(0.2, 2.0)));
            else if ((r >= 4.0) && (r < 5.0))
                node = CappedCylinder::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 5.0) && (r < 6.0))
                node = Cylinder::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 6.0) && (r < 7.0))
                node = CappedCone::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 7.0) && (r < 8.0))
                node = Cone::create(random(0.2, 1.0), random(0.2, 1.0));
            else
                node = Dodecahedron::create(random(0.2, 1.0));

            auto material = Material::create();
            material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));
            material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

            node->setPos(glm::vec3(random(-20.0, 20.0), random(-20.0, 20.0), random(-20.0, 20.0)));
            node->setMaterial(material);

            this->add(node);
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

    auto window = ExampleWindow::create(800, 800, "Example 2");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
```