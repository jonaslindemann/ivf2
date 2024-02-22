#include <ivfui/glfw_window.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#undef GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>

#include <iostream>
#include <ivfui/glfw_window.h>

using namespace std;
using namespace ivfui;

std::shared_ptr<GLFWWindow> GLFWWindow::create(int width, int height, const std::string title, GLFWmonitor *monitor,
                                               GLFWwindow *shared)
{
    return GLFWWindowPtr(new GLFWWindow(width, height, title, monitor, shared));
}

GLFWWindow::GLFWWindow(int width, int height, const std::string title, GLFWmonitor *monitor, GLFWwindow *shared)
    : m_width(width), m_height(height), m_title(title), m_mouseButton(-1), m_mouseAction(-1), m_mouseMods(-1),
      m_mouseX(-1), m_mouseY(-1), m_currentKey(-1), m_altDown(false), m_ctrlDown(false), m_shiftDown(false),
      m_escQuit(true), m_enabled(true), m_runSetup(true), m_lastError(0), m_frameCount(0), m_frameTime(0.0), m_t0(0.0),
      m_t1(0.0), m_monitor(monitor), m_sharedWindow(shared)
{
    m_window = glfwCreateWindow(width, height, title.c_str(), monitor, shared);

    if (!m_window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    this->makeCurrent();

    if (!gladLoadGL())
        exit(EXIT_FAILURE);

    m_uiRenderer = UiRenderer::create(m_window);
}

GLFWWindow::~GLFWWindow()
{
    if (m_window)
        glfwDestroyWindow(m_window);
}

void GLFWWindow::makeCurrent()
{
    if (m_window) {
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);
    }
}

bool GLFWWindow::isClosing()
{
    if (m_window)
        return glfwWindowShouldClose(m_window);
    else
        return true;
}

void GLFWWindow::close()
{
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void GLFWWindow::swapBuffers()
{
    if (m_window)
        glfwSwapBuffers(m_window);
}

void GLFWWindow::destroy()
{
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

GLFWwindow *GLFWWindow::ref()
{
    return m_window;
}

int GLFWWindow::mouseButton()
{
    return m_mouseButton;
}

int GLFWWindow::mouseAction()
{
    return m_mouseAction;
}

int GLFWWindow::mouseMods()
{
    return m_mouseMods;
}

int GLFWWindow::mouseX()
{
    return m_mouseX;
}

int GLFWWindow::mouseY()
{
    return m_mouseY;
}

bool GLFWWindow::isAnyMouseButtonDown()
{
    return ((m_mouseButton == GLFW_MOUSE_BUTTON_1) || (m_mouseButton == GLFW_MOUSE_BUTTON_2) ||
            (m_mouseButton == GLFW_MOUSE_BUTTON_3)) &&
           (m_mouseAction == GLFW_PRESS);
}

bool GLFWWindow::isShiftDown()
{
    return m_shiftDown;
}

bool GLFWWindow::isCtrlDown()
{
    return m_ctrlDown;
}

bool GLFWWindow::isAltDown()
{
    return m_altDown;
}

bool GLFWWindow::useEscQuit()
{
    return m_escQuit;
}

void GLFWWindow::setUseEscQuit(bool flag)
{
    m_escQuit = flag;
}

int GLFWWindow::width()
{
    if (m_window) {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        return m_width;
    }
    else
        return -1;
}

int GLFWWindow::height()
{
    if (m_window) {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        return m_height;
    }
    else
        return -1;
}

void GLFWWindow::getSize(int &width, int &height)
{
    if (m_window) {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        width = m_width;
        height = m_height;
    }
    else {
        width = -1;
        height = -1;
    }
}

void GLFWWindow::setWindowIcon(const std::string filename)
{
    GLFWimage images[1];
    images[0].pixels = stbi_load(filename.c_str(), &images[0].width, &images[0].height, 0, 4); // rgba channels
    glfwSetWindowIcon(m_window, 1, images);
    stbi_image_free(images[0].pixels);
}

void GLFWWindow::maximize()
{
    glfwMaximizeWindow(m_window);
}

void GLFWWindow::enable()
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_enabled = true;
}

void GLFWWindow::disable()
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_enabled = false;
}

bool GLFWWindow::isEnabled()
{
    return m_enabled;
}

void GLFWWindow::draw()
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    this->makeCurrent();

    auto result = 0;

    if (m_runSetup) {
        result = this->doSetup();
        setError(result);
        if (result != 0)
            this->close(); // close window if setup failed
        m_runSetup = false;
    }

    m_uiRenderer->beginFrame();
    this->doDrawUi();
    if ((!m_uiRenderer->wantCaptureMouse()) && (!m_uiRenderer->wantCaptureKeyboard()))
        this->doUpdateOtherUi();
    m_uiRenderer->endFrame();

    if (m_enabled && (result == 0))
        this->doDraw();

    m_uiRenderer->draw();

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    this->swapBuffers();
}

void GLFWWindow::doKey(int key, int scancode, int action, int mods)
{
    m_shiftDown = false;
    m_ctrlDown = false;
    m_altDown = false;

    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_shiftDown = true;

    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        m_ctrlDown = true;

    if (glfwGetKey(m_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        m_altDown = true;

    onKey(key, scancode, action, mods);
}

void GLFWWindow::doMousePosition(double x, double y)
{
    m_mouseX = int(x);
    m_mouseY = int(y);
    onMousePosition(x, y);
}

void GLFWWindow::doMouseButton(int button, int action, int mods)
{
    m_mouseButton = button;
    m_mouseAction = action;
    m_mouseMods = mods;
    onMouseButton(button, action, mods);
}

void GLFWWindow::doResize(int width, int height)
{
    onResize(width, height);
}

int ivfui::GLFWWindow::lastError()
{
    return m_lastError;
}
void GLFWWindow::doDraw()
{
    int width, height;

    this->getSize(width, height);

    const float ratio = width / (float)height;

    glViewport(0, 0, width, height);

    m_t0 = glfwGetTime();
    onDraw();
    m_t1 = glfwGetTime();
    m_frameCount++;
    m_frameTime = m_t1 - m_t0;
}

void ivfui::GLFWWindow::doDrawUi()
{
    onDrawUi();
}

void ivfui::GLFWWindow::doUpdateOtherUi()
{
    onUpdateOtherUi();
}

void ivfui::GLFWWindow::setError(int error)
{
    m_lastError = error;
}

double ivfui::GLFWWindow::frameTime() const
{
    return m_frameTime;
}

int ivfui::GLFWWindow::frameCount() const
{
    return m_frameCount;
}

double ivfui::GLFWWindow::elapsedTime() const
{
    return glfwGetTime();
}

void ivfui::GLFWWindow::clearError()
{
    m_lastError = 0;
}

int GLFWWindow::doSetup()
{
    return onSetup();
}

void GLFWWindow::onKey(int key, int scancode, int action, int mods)
{
}

void GLFWWindow::onMousePosition(double x, double y)
{
}

void GLFWWindow::onMouseButton(int button, int action, int mods)
{
}

void GLFWWindow::onResize(int width, int height)
{
}

void GLFWWindow::onDraw()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void ivfui::GLFWWindow::onDrawUi()
{
}

void ivfui::GLFWWindow::onUpdateOtherUi()
{
}

int GLFWWindow::onSetup()
{
    return 0;
}
