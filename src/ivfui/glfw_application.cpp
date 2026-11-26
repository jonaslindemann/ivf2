#include <ivfui/glfw_application.h>

#include <ivf/logger.h>

#include <glad/glad.h>

#include <iostream>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

using namespace ivf;
using namespace ivfui;

static void error_callback(int code, const char *description)
{
    // GLFWWindow probes for the newest OpenGL version; "unavailable" is expected there.
    if (code == GLFW_VERSION_UNAVAILABLE)
        logDebug(std::string("GLFW: ") + description, "GLFW");
    else
        logError("GLFW error " + std::to_string(code) + ": " + description, "GLFW");
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->makeCurrent(); // managers and GL calls in user callbacks refer to this window

    if (win->useEscQuit()) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            win->close();
    }

    win->doKey(key, scancode, action, mods);
}

static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->makeCurrent(); // managers and GL calls in user callbacks refer to this window
    win->doMouseButton(button, action, mods);
}

static void mouse_pos_callback(GLFWwindow *window, double x, double y)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->makeCurrent(); // managers and GL calls in user callbacks refer to this window
    win->doMousePosition(x, y);
}

static void window_resize_callback(GLFWwindow *window, int width, int height)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->makeCurrent(); // managers and GL calls in user callbacks refer to this window
    win->doResize(width, height);
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto win = GLFWWindowTracker::instance()->get(window);
    win->makeCurrent(); // managers and GL calls in user callbacks refer to this window
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
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        logError("Failed to initialize GLFW", "GLFWApplication");
        exit(EXIT_FAILURE);
    }
}

GLFWApplication::~GLFWApplication()
{
    logInfo("Terminating GLFW", "GLFWApplication");

    // Release the windows while GLFW and their GL contexts still exist. A window held only
    // by the application is destroyed here, together with its scene and the scene's GL
    // objects; deleting GL objects after glfwTerminate() crashes on Linux, where GLFW
    // unloads libGL. Windows still referenced elsewhere at least release their context.
    GLFWWindowTracker::instance()->clear();
    for (auto &window : m_windows)
    {
        window->makeCurrent();
        if (window.use_count() == 1)
            window.reset();
        else
            window->destroy();
    }
    m_windows.clear();

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

#ifdef __EMSCRIPTEN__
    // The browser calls frame() before each repaint (requestAnimationFrame) and delivers the
    // input events in between. simulate_infinite_loop: main() is left without running
    // destructors, so the application and its windows live on with the page.
    emscripten_set_main_loop_arg(
        [](void *app) {
            if (!static_cast<GLFWApplication *>(app)->frame())
                emscripten_cancel_main_loop();
        },
        this, 0, true);
    return 0;
#else
    while (this->frame())
        this->pollEvents();
    return m_error;
#endif
}

bool GLFWApplication::frame()
{
    bool hasOpenWindows = false;

    for (auto &window : m_windows)
    {
        if (window->lastError() != 0)
            m_error = window->lastError();

        if (!window->isClosing())
        {
            hasOpenWindows = true;
#ifdef __EMSCRIPTEN__
            // The canvas is sized by the page (CSS); give its drawing buffer the same size.
            double cssWidth = 0.0, cssHeight = 0.0;
            emscripten_get_element_css_size("#canvas", &cssWidth, &cssHeight);
            int width = 0, height = 0;
            glfwGetWindowSize(window->ref(), &width, &height);
            if (cssWidth >= 1.0 && cssHeight >= 1.0 &&
                (static_cast<int>(cssWidth) != width || static_cast<int>(cssHeight) != height))
                glfwSetWindowSize(window->ref(), static_cast<int>(cssWidth), static_cast<int>(cssHeight));
#endif
            window->draw();
        }
    }

    return hasOpenWindows;
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

void GLFWWindowTracker::clear()
{
    m_windowMap.clear();
}
