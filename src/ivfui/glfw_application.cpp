#include <ivfui/glfw_application.h>

#include <ivf/logger.h>

#include <glad/glad.h>

#include <iostream>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace ivf;
using namespace ivfui;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto win = GLFWWindowTracker::instance()->get(window);

    if (win->useEscQuit()) {
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

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->doScroll(xoffset, yoffset);
}

static void window_close_callback(GLFWwindow *window)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    if (win)
        win->close();
}

GLFWApplication::GLFWApplication()
{
    logInfo("Initializing GLFW", "GLFWApplication");

    if (!glfwInit())
    {
        logError("Failed to initialize GLFW", "GLFWApplication");
        exit(EXIT_FAILURE);
    }
}

GLFWApplication::~GLFWApplication()
{
    logInfo("Terminating GLFW", "GLFWApplication");

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
    logInfo("Adding GLFW window to application", "GLFWApplication");

    m_windows.push_back(window);

    logInfo("Setting GLFW callbacks for window", "GLFWApplication");

    glfwSetKeyCallback(window->ref(), key_callback);
    glfwSetMouseButtonCallback(window->ref(), mouse_callback);
    glfwSetCursorPosCallback(window->ref(), mouse_pos_callback);
    glfwSetWindowSizeCallback(window->ref(), window_resize_callback);
    glfwSetScrollCallback(window->ref(), scroll_callback);
    glfwSetWindowCloseCallback(window->ref(), window_close_callback);

    ImGui_ImplGlfw_InstallCallbacks(window->ref());

    GLFWWindowTracker::instance()->addWindow(window);
}

int GLFWApplication::loop()
{
    logInfo("Starting main application loop", "GLFWApplication");

    int anyError = 0;

    while (true) {
        bool hasOpenWindows = false;

        for (auto window : m_windows) {
            if (window->lastError() != 0)
                anyError = window->lastError();

            if (!window->isClosing()) {
                hasOpenWindows = true;
                window->draw();
            }
        }

        if (!hasOpenWindows)
            break;

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
