#include <ivfui/glfw_application.h>

#include <glad/glad.h>

#include <iostream>

using namespace ivfui;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto win = GLFWWindowTracker::instance()->get(window);

    if (win->useEscQuit())
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            win->close();
    }

    win->doKey(key, scancode, action, mods);
}

static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->doMouseButton(button, action, mods);
}

static void mouse_pos_callback(GLFWwindow *window, double x, double y)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->doMousePosition(x, y);
}

static void window_resize_callback(GLFWwindow *window, int width, int height)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->doResize(width, height);
}

GLFWApplication::GLFWApplication()
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    // if (!gladLoadGL())
    //     exit(EXIT_FAILURE);
}

GLFWApplication::~GLFWApplication()
{
    for (auto window : m_windows)
        window->destroy();

    glfwTerminate();
}

std::shared_ptr<GLFWApplication> GLFWApplication::create()
{
    return std::shared_ptr<GLFWApplication>(new GLFWApplication());
}

void GLFWApplication::addWindow(GLFWWindowPtr window)
{
    m_windows.push_back(window);

    glfwSetKeyCallback(window->ref(), key_callback);
    glfwSetMouseButtonCallback(window->ref(), mouse_callback);
    glfwSetCursorPosCallback(window->ref(), mouse_pos_callback);
    glfwSetWindowSizeCallback(window->ref(), window_resize_callback);

    GLFWWindowTracker::instance()->addWindow(window);
}

int GLFWApplication::loop()
{
    bool shouldClose = false;

    int anyError = 0;

    while (!shouldClose)
    {
        for (auto window : m_windows)
        {
            if (window->lastError() != 0)
                anyError = window->lastError();

            if (!window->isClosing())
            {
                window->draw();
                shouldClose = false;
            }
            else
                shouldClose = true;
        }

        this->pollEvents();
    }
    return anyError;
}

void GLFWApplication::pollEvents()
{
    glfwPollEvents();
}

void GLFWApplication::hint(int hint, int value)
{
    glfwWindowHint(hint, value);
}

void GLFWWindowTracker::addWindow(GLFWWindowPtr window)
{
    m_windowMap[window->ref()] = window;
}

GLFWWindowPtr GLFWWindowTracker::get(GLFWwindow *window)
{
    return m_windowMap[window];
}
