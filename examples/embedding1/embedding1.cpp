/**
 * @file embedding1.cpp
 * @brief Using ivf inside an application that owns its own OpenGL context.
 * @ingroup general_examples
 *
 * This example does not use ivfui. It creates a plain GLFW window, just like a
 * Qt (QOpenGLWidget), SDL or any other application would create its OpenGL
 * context, and hosts ivf in it:
 *
 *  1. make the host's GL context current
 *  2. create an ivf::RenderContext, passing the host's GL function loader
 *  3. make it current and call setupDefaults()
 *  4. build and draw a scene graph; set camera matrices via the context's TransformManager
 *  5. release the scene and the RenderContext while the GL context is still current
 *
 * Only steps 1 and the window/event code are GLFW specific.
 */

#include <GLFW/glfw3.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>

#include <cmath>
#include <cstdlib>

int main()
{
    // ---- Host side: create a window and an OpenGL context (here with GLFW) ----
    if (!glfwInit())
        return EXIT_FAILURE;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // required for core profiles on macOS
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(1024, 768, "Embedding ivf in a host application", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ---- ivf side: one RenderContext per OpenGL context ----
    // Qt: QOpenGLContext::currentContext()->getProcAddress, SDL: SDL_GL_GetProcAddress, ...
    auto ctx = ivf::RenderContext::create(reinterpret_cast<ivf::GLLoadProc>(glfwGetProcAddress));
    if (!ctx)
        return EXIT_FAILURE;

    ctx->makeCurrent();
    if (!ctx->setupDefaults()) // default shaders, font and light
        return EXIT_FAILURE;

    glEnable(GL_DEPTH_TEST);

    // ---- Scene ----
    auto scene = ivf::CompositeNode::create();
    scene->add(ivf::Axis::create());

    auto material = ivf::Material::create();
    material->setDiffuseColor({0.9f, 0.5f, 0.2f, 1.0f});

    auto shape = ivf::RoundedBox::create(glm::vec3(2.0f, 1.0f, 1.0f));
    shape->setMaterial(material);
    scene->add(shape);

    auto xfm = ctx->transforms();

    // ---- Host render loop ----
    while (!glfwWindowShouldClose(window))
    {
        // A host with several GL contexts makes the matching ivf context current
        // together with its GL context before rendering.
        ctx->makeCurrent();

        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Camera: projection and an orbiting view
        const float t = static_cast<float>(glfwGetTime());
        xfm->enableProjectionMatrix();
        xfm->identity();
        xfm->perspective(45.0f, height > 0 ? float(width) / float(height) : 1.0f, 0.1f, 100.0f);
        xfm->enableViewMatrix();
        xfm->identity();
        xfm->lookAt(glm::vec3(8.0f * std::cos(t * 0.5f), 4.0f, 8.0f * std::sin(t * 0.5f)), glm::vec3(0.0f));
        xfm->enableModelMatrix();
        xfm->identity();

        shape->setRotAxis({1.0f, 1.0f, 0.0f});
        shape->setRotAngle(t * 40.0f);

        glClearColor(0.07f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene->draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Shutdown: release GL objects while the GL context is still current ----
    scene.reset();
    shape.reset();
    material.reset();
    ctx.reset();

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
