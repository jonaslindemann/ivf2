#include <ivfui/glfw_window.h>

#include <ivf/gl_debug.h>
#include <ivf/image_io.h>

#undef GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <filesystem>
#include <iostream>
#include <vector>

#include <ivf/logger.h>
#include <ivfui/glfw_window.h>
#include <ivf/shader_manager.h>


namespace {

// Value of an environment variable, or nullptr if unset or empty.
const char *envValue(const char *name)
{
    const char *value = std::getenv(name);
    return (value != nullptr && *value != '\0') ? value : nullptr;
}

// True if the environment variable is set to something other than "0".
bool envFlag(const char *name)
{
    const char *value = envValue(name);
    return value != nullptr && std::string_view(value) != "0";
}

// OpenGL debug context: on in debug builds, IVF2_GL_DEBUG=1/0 forces it on/off.
bool debugContextRequested()
{
    if (const char *value = envValue("IVF2_GL_DEBUG"))
        return std::string_view(value) != "0";
#ifndef NDEBUG
    return true;
#else
    return false;
#endif
}

int g_windowCount = 0; // numbers IVF2_SCREENSHOT files when there are several windows

} // namespace

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
      m_escQuit(true), m_enabled(true), m_runSetup(true), m_closeNotified(false), m_lastError(0), m_frameCount(0), m_frameTime(0.0), m_t0(0.0),
      m_t1(0.0), m_lastFrameTime(0.0), m_monitor(monitor), m_sharedWindow(shared),
      m_scrollX(0.0), m_scrollY(0.0),
      m_prevMouseX(-1.0), m_prevMouseY(-1.0), m_isDragging(false),
      m_lastClickTime(-1.0), m_lastClickButton(-1)
{
    // Core profile; the version is chosen below (stock shaders need GLSL 4.00).
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    const bool debugContext = debugContextRequested();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debugContext ? GLFW_TRUE : GLFW_FALSE);

    // Automation for scripted runs and CI (see class documentation)
    m_hidden = envFlag("IVF2_HIDE_WINDOWS");
    if (const char *frames = envValue("IVF2_EXIT_AFTER_FRAMES"))
        m_autoExitFrames = std::max(0, std::atoi(frames));
    if (const char *shot = envValue("IVF2_SCREENSHOT"))
    {
        std::filesystem::path path(shot);
        if (g_windowCount > 0) // second and later windows: name_2.png, name_3.png, ...
            path = path.parent_path() /
                   (path.stem().string() + "_" + std::to_string(g_windowCount + 1) + path.extension().string());
        m_autoScreenshot = path.string();
    }
    ++g_windowCount;
    if (m_hidden)
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Request the newest OpenGL first: drivers return exactly the requested version, and
    // 4.3+ provides debug output (and compute shaders). macOS supports at most 4.1.
    constexpr int versions[][2] = {{4, 6}, {4, 3}, {4, 1}};
    for (const auto &version : versions)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version[0]);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version[1]);
        m_window = glfwCreateWindow(width, height, title.c_str(), monitor, shared);
        if (m_window)
            break;
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE); // don't leak the hint to other windows

    if (!m_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    this->makeCurrent();

    // Every GL context gets its own ivf RenderContext, which also loads the GL functions.
    m_renderContext = ivf::RenderContext::create(reinterpret_cast<ivf::GLLoadProc>(glfwGetProcAddress));
    if (!m_renderContext)
        exit(EXIT_FAILURE);
    m_renderContext->makeCurrent();

    // Route driver messages (errors, undefined behaviour, performance hints) to the Logger.
    if (debugContext && !ivf::enableGLDebugOutput())
        ivf::logInfo("OpenGL debug output not available (requires OpenGL 4.3)", "GLFWWindow");

    m_uiRenderer = UiRenderer::create(m_window);
}

GLFWWindow::~GLFWWindow()
{
    this->destroy();
}

void GLFWWindow::makeCurrent()
{
    if (m_window)
    {
        if (glfwGetCurrentContext() != m_window)
            glfwMakeContextCurrent(m_window);

        // Exports render as fast as possible instead of at the display refresh rate
        const int interval = m_exporting ? 0 : 1;
        if (interval != m_swapInterval)
        {
            glfwSwapInterval(interval);
            m_swapInterval = interval;
        }
    }

    if (m_renderContext)
        m_renderContext->makeCurrent();

    if (m_uiRenderer)
        m_uiRenderer->makeCurrent();
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

    if (!m_closeNotified)
    {
        m_closeNotified = true;
        doClose();
    }
}

void GLFWWindow::swapBuffers()
{
    if (m_window)
        glfwSwapBuffers(m_window);
}

void GLFWWindow::destroy()
{
    if (m_window)
    {
        // Release ImGui and ivf resources while this window's GL context is still current.
        this->makeCurrent();

        if (m_uiRenderer)
            m_uiRenderer->shutdown();

        m_renderContext.reset();

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
    if (m_window)
    {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        return m_width;
    }
    else
        return -1;
}

int GLFWWindow::height()
{
    if (m_window)
    {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        return m_height;
    }
    else
        return -1;
}

void GLFWWindow::getSize(int &width, int &height)
{
    if (m_window)
    {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        width = m_width;
        height = m_height;
    }
    else
    {
        width = -1;
        height = -1;
    }
}

void GLFWWindow::setWindowIcon(const std::string filename)
{
    GLFWimage images[1];
    images[0].pixels = ivf::loadImage(filename.c_str(), &images[0].width, &images[0].height, nullptr, 4); // rgba
    if (!images[0].pixels)
    {
        ivf::logError("Could not load window icon: " + filename, "GLFWWindow");
        return;
    }
    glfwSetWindowIcon(m_window, 1, images);
    ivf::freeImage(images[0].pixels);
}

void GLFWWindow::maximize()
{
    // Maximizing would show a window created hidden (IVF2_HIDE_WINDOWS).
    if (!m_hidden)
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
    // Make sure only a single thread is drawing

    const std::lock_guard<std::mutex> lock(m_mutex);

    // Make this window the current context

    this->makeCurrent();

    // IVF2_SCREENSHOT: capture the last frame before IVF2_EXIT_AFTER_FRAMES closes the window
    if (!m_autoScreenshot.empty() && m_autoExitFrames > 0 && m_drawnFrames == m_autoExitFrames - 1)
        this->saveScreenshot(m_autoScreenshot, true);

    // Frame time: wall-clock delta since the previous draw(), or the fixed step
    // when exporting frames, so animation is deterministic and independent of
    // how long a frame takes to render.
    double now = glfwGetTime();
    if (m_exportPending)
    {
        // First exported frame: switch to fixed steps at the requested time
        m_exportPending = false;
        m_exporting = true;
        m_previousFixedTimeStep = m_fixedTimeStep;
        const double step = 1.0 / m_recordFps;
        m_simTime = m_exportStartTime >= 0.0 ? m_exportStartTime : m_simTime + step;
        m_fixedTimeStep = step;
        m_fixedStepStarted = true;
        m_frameTime = step;
        this->onFrameExportStarted();
    }
    else if (m_fixedTimeStep > 0.0)
    {
        // The frame that switched to fixed steps keeps its time; later frames advance
        if (m_fixedStepStarted)
            m_simTime += m_fixedTimeStep;
        m_fixedStepStarted = true;
        m_frameTime = m_fixedTimeStep;
    }
    else
    {
        if (m_lastFrameTime > 0.0)
            m_frameTime = now - m_lastFrameTime;
        m_simTime = now - m_timeOffset;
    }
    m_lastFrameTime = now;

    auto result = 0;

    if (m_runSetup)
    {
        // Run window setup routine

        result = this->doSetup();

        setError(result);
        if (result != 0)
        {
            // Setup failed (the reason is logged): close without updating or drawing,
            // since onUpdate()/onDraw() may use objects onSetup() did not create.
            m_runSetup = false;
            this->close();
            return;
        }

        // If maximized called we need to do a resize

        glfwGetWindowSize(m_window, &m_width, &m_height);
        this->doResize(m_width, m_height);

        // Only run setup once.

        m_runSetup = false;
    }

    // Update scene graph here

    this->doUpdate();

    // Create user interfacee

    m_uiRenderer->beginFrame();
    this->doDrawUi();
    if ((!m_uiRenderer->wantCaptureMouse()) && (!m_uiRenderer->wantCaptureKeyboard()))
        this->doUpdateOtherUi();
    m_uiRenderer->endFrame();

    // Render scene

    this->beginGpuTimer();

    if (m_enabled && (result == 0))
    {
        if (auto shaderMgr = ivf::ShaderManager::instance(); shaderMgr->currentProgram())
            shaderMgr->apply();
        this->doDraw();
    }

    // Capture the scene without the interface (before the back buffer is swapped)

    this->captureFrames(false);

    // Render user interfaec

    m_uiRenderer->draw();

    // Tell derived class to do any final rendering

    this->doDrawComplete();

    this->endGpuTimer();

    // Capture with the interface

    this->captureFrames(true);

    // Window/viewport handling

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    // Swap buffers

    this->swapBuffers();

    // IVF2_EXIT_AFTER_FRAMES
    ++m_drawnFrames;
    if (m_autoExitFrames > 0 && m_drawnFrames >= m_autoExitFrames)
        this->close();

    // An export with a frame count ends after its last frame
    if (m_exporting && m_exportFrameCount > 0 && m_recordedFrames >= m_exportFrameCount)
    {
        bool closeWhenDone = m_exportCloseWhenDone;
        this->finishExport();
        if (closeWhenDone)
            this->close();
    }
}

void ivfui::GLFWWindow::drawScene()
{
    this->doDraw();
}

void ivfui::GLFWWindow::scaleAllSizes(double scale)
{
    m_uiRenderer->scaleAllSizes(float(scale));
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
    double dx = (m_prevMouseX >= 0.0) ? x - m_prevMouseX : 0.0;
    double dy = (m_prevMouseY >= 0.0) ? y - m_prevMouseY : 0.0;
    m_prevMouseX = x;
    m_prevMouseY = y;

    m_mouseX = int(x);
    m_mouseY = int(y);
    onMousePosition(x, y);

    if (m_isDragging && (dx != 0.0 || dy != 0.0))
        onMouseDrag(x, y, dx, dy, m_mouseButton);
}

void GLFWWindow::doMouseButton(int button, int action, int mods)
{
    m_mouseButton = button;
    m_mouseAction = action;
    m_mouseMods = mods;

    if (action == GLFW_PRESS) {
        m_isDragging = true;

        double now = glfwGetTime();
        if (button == m_lastClickButton && (now - m_lastClickTime) <= k_doubleClickInterval)
            onDoubleClick(button, mods);
        m_lastClickTime = now;
        m_lastClickButton = button;
    } else if (action == GLFW_RELEASE) {
        m_isDragging = false;
    }

    onMouseButton(button, action, mods);
}

void GLFWWindow::doResize(int width, int height)
{
    onResize(width, height);
}

void ivfui::GLFWWindow::doUpdate()
{
    onUpdate();
}

void GLFWWindow::doClose()
{
    onClose();
}

int ivfui::GLFWWindow::lastError() const
{
    return m_lastError;
}

void GLFWWindow::doDraw()
{
    int width, height;

    this->getSize(width, height);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    m_t0 = glfwGetTime();
    onDraw();
    m_t1 = glfwGetTime();
    m_frameCount++;
}

void ivfui::GLFWWindow::doDrawUi()
{
    onDrawUi();
}

void ivfui::GLFWWindow::doDrawComplete()
{
    onDrawComplete();
}

void ivfui::GLFWWindow::doUpdateOtherUi()
{
    onUpdateOtherUi();
}

void ivfui::GLFWWindow::doPreSetup()
{}

void ivfui::GLFWWindow::setError(int error)
{
    m_lastError = error;
}

double ivfui::GLFWWindow::frameTime() const
{
    return m_frameTime;
}

double ivfui::GLFWWindow::gpuFrameTime() const
{
    return m_gpuFrameTime;
}

void ivfui::GLFWWindow::beginGpuTimer()
{
    // Query objects are freed with the context
    if (m_gpuQueries[0] == 0)
        glGenQueries(k_gpuQueryCount, m_gpuQueries);

    // Collect finished results without waiting for the GPU
    for (int i = 0; i < k_gpuQueryCount; i++)
    {
        if (!m_gpuQueryPending[i])
            continue;

        GLint available = 0;
        glGetQueryObjectiv(m_gpuQueries[i], GL_QUERY_RESULT_AVAILABLE, &available);
        if (!available)
            continue;

        GLuint64 elapsed = 0;
        glGetQueryObjectui64v(m_gpuQueries[i], GL_QUERY_RESULT, &elapsed);
        m_gpuQueryPending[i] = false;

        const double seconds = double(elapsed) * 1e-9;
        m_gpuFrameTime = (m_gpuFrameTime > 0.0) ? 0.9 * m_gpuFrameTime + 0.1 * seconds : seconds;
    }

    // All queries still in flight: skip timing this frame rather than stall
    m_gpuTimerRunning = !m_gpuQueryPending[m_gpuQueryIndex];
    if (m_gpuTimerRunning)
        glBeginQuery(GL_TIME_ELAPSED, m_gpuQueries[m_gpuQueryIndex]);
}

void ivfui::GLFWWindow::endGpuTimer()
{
    if (!m_gpuTimerRunning)
        return;

    glEndQuery(GL_TIME_ELAPSED);
    m_gpuQueryPending[m_gpuQueryIndex] = true;
    m_gpuQueryIndex = (m_gpuQueryIndex + 1) % k_gpuQueryCount;
    m_gpuTimerRunning = false;
}

int ivfui::GLFWWindow::frameCount() const
{
    return m_frameCount;
}

double ivfui::GLFWWindow::elapsedTime() const
{
    return m_simTime;
}

void GLFWWindow::setFixedTimeStep(double dt)
{
    dt = std::max(dt, 0.0);
    if (dt == m_fixedTimeStep)
        return;

    if (dt == 0.0)
    {
        // Continue real time from the current simulation time
        m_timeOffset = glfwGetTime() - m_simTime;
    }
    else if (m_fixedTimeStep == 0.0)
    {
        m_fixedStepStarted = false;
    }
    m_fixedTimeStep = dt;
}

void ivfui::GLFWWindow::clearError()
{
    m_lastError = 0;
}

int GLFWWindow::doSetup()
{
    // Run pre-setup routine
    this->doPreSetup();
    auto retval = onSetup();
    // Run post-setup routine
    this->doPostSetup();

    return retval;
}

void ivfui::GLFWWindow::doPostSetup()
{}

void GLFWWindow::onKey(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{}

void GLFWWindow::onMousePosition(double /*x*/, double /*y*/)
{}

void GLFWWindow::onMouseButton(int /*button*/, int /*action*/, int /*mods*/)
{}

void GLFWWindow::onResize(int /*width*/, int /*height*/)
{}

void ivfui::GLFWWindow::onUpdate()
{}

void GLFWWindow::onClose()
{}

void GLFWWindow::onDraw()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void ivfui::GLFWWindow::onDrawUi()
{}

void ivfui::GLFWWindow::onDrawComplete()
{}

void ivfui::GLFWWindow::onUpdateOtherUi()
{}

void GLFWWindow::doScroll(double xoffset, double yoffset)
{
    m_scrollX = xoffset;
    m_scrollY = yoffset;
    onScroll(xoffset, yoffset);
}

void ivfui::GLFWWindow::onScroll(double /*xoffset*/, double /*yoffset*/)
{}

void ivfui::GLFWWindow::onMouseDrag(double /*x*/, double /*y*/, double /*dx*/, double /*dy*/, int /*button*/)
{}

void ivfui::GLFWWindow::onDoubleClick(int /*button*/, int /*mods*/)
{}

int GLFWWindow::onSetup()
{
    return 0;
}

namespace {

std::string framePath(const std::string &directory, int index)
{
    char name[32];
    std::snprintf(name, sizeof(name), "frame_%06d.png", index);
    return (std::filesystem::path(directory) / name).string();
}

bool ensureDirectory(const std::string &directory)
{
    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    if (ec)
        ivf::logError("Could not create directory " + directory + ": " + ec.message(), "GLFWWindow");
    return !ec;
}

} // namespace

std::vector<unsigned char> GLFWWindow::readFramePixels(int &width, int &height)
{
    getSize(width, height);
    if (width <= 0 || height <= 0)
        return {};

    // Read the finished frame from the back buffer, before it is swapped
    std::vector<unsigned char> pixels(size_t(width) * height * 3);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    return pixels;
}

void GLFWWindow::captureFrames(bool uiDrawn)
{
    const bool screenshot = !m_screenshotPath.empty() && m_screenshotIncludeUi == uiDrawn;

    // Number of sequence frames this rendered frame stands for
    int copies = 0;
    if (m_exporting && m_recordIncludeUi == uiDrawn)
    {
        copies = 1;
    }
    else if (m_recording && m_recordIncludeUi == uiDrawn)
    {
        // Real time: repeat the frame when rendering is slower than the capture rate
        const double interval = 1.0 / m_recordFps;
        m_recordAccum += m_frameTime;
        while (m_recordAccum >= interval)
        {
            ++copies;
            m_recordAccum -= interval;
        }
    }

    if (!screenshot && copies == 0)
        return;

    int width, height;
    auto pixels = readFramePixels(width, height);
    if (pixels.empty())
        return;

    if (!m_frameWriter)
        m_frameWriter = std::make_unique<FrameWriter>();

    if (screenshot)
    {
        m_frameWriter->write(m_screenshotPath, width, height, copies > 0 ? pixels : std::move(pixels));
        m_screenshotPath.clear();
    }

    for (int i = 0; i < copies; ++i)
    {
        auto frame = (i + 1 < copies) ? pixels : std::move(pixels);
        m_frameWriter->write(framePath(m_recordPath, m_recordFrameIdx++), width, height, std::move(frame));
        ++m_recordedFrames;
    }
}

void GLFWWindow::saveScreenshot(const std::string &path, bool includeUi)
{
    m_screenshotPath = path;
    m_screenshotIncludeUi = includeUi;
}

void GLFWWindow::startRecording(const std::string &directory, int fps, int startFrame, bool includeUi)
{
    stopRecording();
    if (!ensureDirectory(directory))
        return;

    m_recordPath = directory;
    m_recordFps = std::max(fps, 1);
    m_recordFrameIdx = startFrame;
    m_recordedFrames = 0;
    m_recordIncludeUi = includeUi;
    m_recordAccum = 1.0 / m_recordFps; // capture the current frame first
    m_recording = true;
}

void GLFWWindow::startFrameExport(const FrameExportSettings &settings)
{
    stopRecording();
    if (!ensureDirectory(settings.directory))
        return;

    m_recordPath = settings.directory;
    m_recordFps = std::max(settings.fps, 1);
    m_recordFrameIdx = settings.startFrame;
    m_recordedFrames = 0;
    m_exportFrameCount = std::max(settings.frameCount, 0);
    m_recordIncludeUi = settings.includeUi;
    m_exportCloseWhenDone = settings.closeWhenDone;
    m_exportStartTime = settings.startTime;

    // Takes effect at the start of the next frame, see draw()
    m_exportPending = true;

    ivf::logInfo("Exporting frames to " + m_recordPath + " at " + std::to_string(m_recordFps) + " fps", "GLFWWindow");
}

void GLFWWindow::finishExport()
{
    m_exportPending = false;
    if (!m_exporting)
        return;

    m_exporting = false;
    setFixedTimeStep(m_previousFixedTimeStep);
    if (m_frameWriter)
        m_frameWriter->finish();

    ivf::logInfo("Exported " + std::to_string(m_recordedFrames) + " frames to " + m_recordPath, "GLFWWindow");
    this->onFrameExportFinished();
}

void GLFWWindow::stopRecording()
{
    finishExport();
    m_recording = false;
    if (m_frameWriter)
        m_frameWriter->finish();
}
