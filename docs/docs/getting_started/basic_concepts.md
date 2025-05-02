# Basic concepts

The Ivf2 library is designed to be easy to use and integrate into your projects. This document provides an overview of the basic concepts and components of the library. It is recommended to read this document before diving into the examples and tutorials.

## Architectural Overview

The Ivf2 library is built around a modular architecture that allows for easy extension and customization. One of the key concepts is to abstract a lot of the OpenGL functionality into higher-level constructs, making it easier to work with. The library is designed to be flexible and extensible, allowing you to create complex applications with minimal effort.

Ivf2 is not designed to be a game engine, but rather a graphics library that provides the necessary parts to implement interactive graphical applications. It is built on top of OpenGL and provides a higher-level interfaces for rendering, resource management, and window management.

The library is also designed in a way that allows you to use OpenGL directly if you need to. This means that you can use Ivf2 to manage your resources and windowing, while still having the flexibility to use OpenGL directly for rendering if needed.

The development of the library has also been a personal project for me to learn more about OpenGL and graphics programming. As such, the library is designed to be easy to use and understand, with a focus on providing a good developer experience. In this process I have also tried design it with modern C++ in mind, using features like smart pointers and RAII to manage resources and avoid memory leaks.

Ivf2 is also designed to be cross-platform and works on Windows, macOS, and Linux. It uses CMake as its build system, making it easy to integrate into your projects.

## Scene Graph System

Ivf2 uses a scene graph system to manage the objects in your scene. The scene graph is a hierarchical structure that represents the objects in your scene and their relationships to each other. It allows you to organize your objects in a way that makes it easy to manage and render them.

The scene graph system in Ivf2 is built around the concept of nodes, which represent the objects in your scene. All nodes in Ivf2 derives from the `Node` and `TransformNode` classes, which provide a common interface for all nodes such as orientation, assigning an id, setting a parent, setting visibility and other properties such as material and texture. Rendering of the node is implemented by derived classed in the `doPreDraw()`, `doDraw()` , and `doPostDraw()` methods. These methods are called when calling the `draw()` method on the node, allowing you to implement custom rendering logic for your nodes. The Node class should not be used directly, but rather as a base class for your own nodes. Position, rotation, and scaling is handled by the derived `Node` class `TransformNode` class. The `TransformNode` class provides methods for setting and getting the position, rotation, and scaling of the node. It also provides methods for converting between world and local coordinates.

There are 4 types of nodes in the Ivf2 scene graph system:

- `MeshNode`: Represents a mesh in the scene. It is used to render 3D models and can be used to create complex scenes with multiple meshes.
- `CompositeNode` / `Transform`: Represents a group of nodes in the scene. It is used to create complex scenes with multiple nodes and can be used to create hierarchical structures.
- `InstanceNode`: Represents an instance of a mesh in the scene. It is used to create multiple instances of a mesh with different transformations and properties.
- `TextNode`: Represents a text in the scene. It is used to render text in the scene and can be used to create complex scenes with multiple text nodes.

## Rendering Pipeline

Ivf2 has no built-in rendering pipeline, but it provides a set of classes and functions that can be used to create your own rendering pipeline. The library provides a set of classes for managing shaders, textures, and materials, as well as a set of functions for rendering objects in the scene.

Rendering with Ivf2 is usually done in to steps. In the first step, the setup stage, consists of initialising the shaders, textures, and other resources needed for rendering. This is usually done in the `onSetup()` method of the `GLFWWindow` class. We will go through the steps of the setup stage in the following sections.

First we initialise a basic shader. Ivf2 provides a basic shader that implements most of the funcitonality required for rendering the Ivf2 nodes. The code below shows how this is done.

```cpp
int onSetup()
{
    // Load basic shaders

    auto shaderMgr = ShaderManager::instance();
    shaderMgr->loadBasicShader();

    // Check for shader compilation errors

    if (shaderMgr->compileLinkErrors())
    {
        cout << "Couldn't compile shaders, exiting..." << endl;
        return -1;
    }
```

!!! note 

    In all the examples in this chapter we have omitted the ivf namespace. The Ivf2 library is designed to be used with the `ivf` namespace, so all classes and functions in the library are prefixed with `ivf::`. In the examples we will use the `using namespace ivf;` directive to avoid having to prefix all classes and functions with `ivf::`. This is not recommended in production code, but it makes the examples easier to read.

The `ShaderManager` singleton class is used to manage the shaders in the library. It provides methods for loading and compiling shaders, as well as for checking for compilation errors. The `ShaderManager` class is a singleton, meaning that there is only one instance of it in the application. The `instance()` method of this class will always return the same instance of the class. The `loadBasicShader()` method loads the basic shader from the Ivf2 library. The `compileLinkErrors()` method checks for compilation errors in the shaders. If there are any errors, it will print them to the console and return true. If there are no errors, it will return false.

In the next step we initialise scene lighting by using the `LightManager` class. The `LightManager` class is used to manage the lights in the scene. It provides methods for adding and removing lights, as well as for updating the lights in the scene. The `LightManager` class is also a singleton accessed by the `instance()` method. The code below shows how to add a light to the scene.

```cpp
    // Setup lighting

    auto lightMgr = LightManager::instance();

    auto pointLight1 = lightMgr->addPointLight();
    pointLight1->setEnabled(true);
    pointLight1->setPosition(glm::vec3(5.0, 5.0, 15.0));

    lightMgr->apply();
```

The `addPointLight()` method adds a point light to the scene. The `setEnabled()` method enables or disables the light. The `setPosition()` method sets the position of the light in the scene. The `apply()` method updates the lights in the shader. The `LightManager` class also provides methods for adding and removing other types of lights, such as directional lights and spotlights.

Next, we setup out scene nodes for rendering. In out example we are using a `CompositeNode` to represent the scene. The `CompositeNode` class is used to create a group of nodes in the scene to be rendered together. It can also be used to create complex scenes with multiple nodes and to create hierarchical structures.  The code below shows how to create a `CompositeNode` and add a `Cube` node to it.

!!! note

    Variables in the examples prefixed with `m_` are member variables of the `GLFWWindow` derived class. 

```cpp
    // Setup scene

    m_scene = CompositeNode::create();

    auto node = Cube::create();

    auto material = Material::create();
    material->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
    material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

    node->setPos(glm::vec3(0.0, 0.0, 0.0));
    node->setMaterial(material);

    m_scene->add(node);
```

In the code above we create a `Cube` node and set its position and material. The `Cube` class is a derived class of the `MeshNode` class, which is used to render 3D models in the scene. The `Material` class is used to set the material properties of the node, such as the diffuse and ambient colors. The `setPos()` method sets the position of the node in the scene. 

Finally, we setup our camera for rendering. The camera is used to view the scene and is usually implemented as a separate class. The code below shows how to create a camera and set its position and orientation.

```cpp
    // Setup camera

    m_camera = Camera::create();
    m_camera->setPosition(glm::vec3(-3.0, 3.0, 3.0));
    m_camera->setTarget(glm::vec3(0.0, 0.0, 0.0));
    m_camera->setUp(glm::vec3(0.0, 1.0, 0.0));
    m_camera->setPerspective(45.0, 0.1f, 100.0f);
    m_camera->setViewport(this->width(), this->height());
```

Finally we are ready to render our scene. The rendering is done in the `onDraw()` method of the `GLFWWindow` class. The code below shows how to render the scene.

```cpp
void onDraw()
{
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_scene->draw();
}
```

This was a high level overview of the rendering pipeline in Ivf2. The idea is to provide easy to use classes and functions that can be used to create a rendering pipeline that fits your needs, without having to worry about the low level details of OpenGL. 

## Window and Application Management

The Ivf2 library provides a set of classes based on GLFW for managing windows and applications. The `GLFWWindow` class is the main class for managing windows and provides methods for creating, destroying, and managing windows. It also provides methods for handling input events, such as keyboard and mouse events.

To implement your own application, you first need to implement a class that derives from the `GLFWWindow` class. In this class you will need to implement/override the `onSetup()`, `onDraw()`, and `onKey()` virtual methods to handle the setup, rendering, and input events for your application. The `onSetup()` method is called once when the window is created and is used to setup the scene and resources needed for rendering. You can expect to have a rendering context available when this method is called. The `onDraw()` method is called when the window needs to be redrawn and is used to render the scene. The `onKey()` method is called when a key is pressed or released and is used to handle input events. The following shows a simple example of a class that derives from the `GLFWWindow` class.

```cpp
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWWindow {
private:
    CompositeNodePtr m_scene;
    CameraPtr m_camera;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Load basic shaders

        auto shaderMgr = ShaderManager::instance();
        shaderMgr->loadBasicShader();

        // Check for shader compilation errors

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Setup lighting

        auto lightMgr = LightManager::instance();

        auto pointLight1 = lightMgr->addPointLight();
        pointLight1->setEnabled(true);
        pointLight1->setPosition(glm::vec3(5.0, 5.0, 15.0));

        lightMgr->apply();

        // Setup scene

        m_scene = CompositeNode::create();

        auto node = Cube::create();

        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
        material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

        node->setPos(glm::vec3(0.0, 0.0, 0.0));
        node->setMaterial(material);

        m_scene->add(node);

        // Setup camera

        m_camera = Camera::create();
        m_camera->setPosition(glm::vec3(-3.0, 3.0, 3.0));
        m_camera->setTarget(glm::vec3(0.0, 0.0, 0.0));
        m_camera->setUp(glm::vec3(0.0, 1.0, 0.0));
        m_camera->setPerspective(45.0, 0.1f, 100.0f);

        m_camera->setViewport(this->width(), this->height());

        return 0;
    }

    void onDraw()
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->draw();
    }

    void onResize(int width, int height)
    {
        m_camera->setViewport(width, height);
    }
};
```

The event loop and window management is handled by the `GLFWApplication` class. This class is a simple wrapper around the GLFW library and provides a simple interface for creating and managing windows. The event loop is implemented in the `loop()` method of the `GLFWApplication` class. The `run()` method creates a window and enters the event loop, handling input events and rendering the scene. It is also in the `GLFWApplication` class that you setup what OpenGL version and attributes you want to use. The code below shows how to create a window and enter the event loop.

```cpp
typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    // Create a FLFW application instance

    auto app = GLFWApplication::create();

    // Set GLFW hints for OpenGL version and profile

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    // Create our example window

    auto window = ExampleWindow::create(800, 800, "Hello, ivf2");
    window->maximize();

    // Add window to application instance

    app->addWindow(window);

    // Enter application loop. This will block until the application is closed.

    return app->loop();
}
```

The `GLFWApplication` class and `GLFWWindow` class are implemented in the `imgui` library. This library also contains the user interface classes for implementing ImGui windows and menus. 

## Resource Management

Ivf2 doesn't implement a resource management system, but it provides a set of classes and functions that can be used to manage resources in your application. The library provides a set of classes for managing shaders, textures, and materials, as well as a set of functions for loading and saving resources.

Shared are used extensively in the library to manage resources. The `std::shared_ptr` class is used to manage the lifetime of objects in the library. This means that you don't have to worry about memory management and can focus on implementing your application logic. The `std::shared_ptr` class is a smart pointer that manages the lifetime of an object and automatically deletes it when it is no longer needed. This means that you don't have to worry about memory leaks and can focus on implementing your application logic.

The main principle is that you use the `onSetup()` method to load and setup your resources, and the `onDraw()` method to render your scene. The `onSetup()` method is called once when the window is created and is used to setup the scene and resources needed for rendering. The main window class will be responsible for maintaining the scene with its attached resources. 

Using a `CompositeNode` as the main scene node is a good way to manage your resources. Materials and textures can be creates and assigned to the nodes in the scene. As they are shared pointers, they will be automatically deleted when they are no longer needed. This means that you don't have to worry about memory management and can focus on implementing your application logic. When the scene is destroyed, all the resources will be automatically deleted. 

## Core Design Patterns

As Ivf2 uses shared pointers extensively, it is important to understand how they work and how to use them correctly. The library uses the RAII (Resource Acquisition Is Initialization) pattern to manage resources. This means that resources are acquired and released in the constructor and destructor of the class. This ensures that resources are always released when they are no longer needed and avoids memory leaks.

To make it easier to use shared pointers, all the classes in the library have a special `create()` method that returns a shared pointer to the class. This method is used to create instances of the class and ensures that the class is always created with a shared pointer. The `create()` method is a static method that returns a shared pointer to the class. You don't have to call `std::make_shared()` yourself, as the `create()` method will do that for you. This makes it easier to create instances of the class and avoids memory leaks. Most of the examples in this documentation will use the `create()` method to create instances of the class. In the following example we create a `Cube` node and set its position and material using the `create()` method.

```cpp
auto node = Cube::create();
auto material = Material::create();

material->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));
node->setPos(glm::vec3(0.0, 0.0, 0.0));
node->setMaterial(material);
```

All classes in the library also provide typedefs for the shared pointers to the class. This makes it easier to use shared pointers and avoids having to type `std::shared_ptr` all the time. The typedefs are usually named `ClassNamePtr`, where `ClassName` is the name of the class. For example, the `Cube` class has a typedef named `CubePtr`, which is a shared pointer to the `Cube` class. You can use this typedef to create shared pointers to the `Cube` class without having to type `std::shared_ptr<Cube>` all the time.

```cpp
typedef std::shared_ptr<Cube> CubePtr;
```

For example a class attribute for a `CompositeNode` can be declared as follows:

```cpp
CompositeNodePtr m_scene;
```

When creating references to classes in the library in most cases you can also use the `auto` keyword to automatically deduce the type of the variable. This makes it easier to write code and avoids having to type the full type of the variable. For example, you can create a shared pointer to a `Cube` class as follows:

```cpp
auto node = Cube::create();
```