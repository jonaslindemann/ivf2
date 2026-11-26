#include <ivfui/ui_manager.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ivfui/ui_font.h>

#include <ivf/config.h>
#include <ivf/render_context.h>

#if IVF2_HAS_IMPLOT
#include <implot.h>
#endif

#include <cstdlib>
#include <string_view>

using namespace ivfui;

UiRenderer::UiRenderer(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();

    // Each window gets its own ImGui context; GLFWWindow makes it current
    // together with the window's GL context and RenderContext.
    m_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_context);
#if IVF2_HAS_IMPLOT
    // ImPlot keeps its own context next to ImGui's, switched together in makeCurrent()
    auto *plot = ImPlot::CreateContext();
    ImPlot::SetCurrentContext(plot);
    m_plotContext = plot;
#endif

    ImGuiIO &io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // Multi-Viewport / Platform Windows (ImGui windows can leave the main window);
    // IVF2_IMGUI_VIEWPORTS=0 turns them off.
    const char *viewports = std::getenv("IVF2_IMGUI_VIEWPORTS");
    if (viewports == nullptr || std::string_view(viewports) != "0")
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // fs::path filename = m_fontPath / fs::path("RopaSans-Regular.ttf");

    io.Fonts->AddFontFromMemoryCompressedTTF(Ivf2Font_compressed_data, Ivf2Font_compressed_size, 22);
    // io.Fonts->AddFontFromFileTTF("fonts/Gidole-Regular.ttf", 22);

    ImGuiStyle &style = ImGui::GetStyle();

    ImGuiStyle newStyle;

    newStyle.FrameRounding = 4;
    newStyle.WindowRounding = 8;
    newStyle.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.5f);
    newStyle.Colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.8f);
    newStyle.Colors[ImGuiCol_TitleBgActive] =
        ImVec4(newStyle.Colors[ImGuiCol_TitleBgActive].x, newStyle.Colors[ImGuiCol_TitleBgActive].y,
               newStyle.Colors[ImGuiCol_TitleBgActive].z, 0.8f);

    newStyle.ScaleAllSizes(1.0);

    io.FontGlobalScale = 1.0;

    style = newStyle;

    // Setup Platform/Renderer backends

    const char *glsl_version = ivf::RenderContext::isGles() ? "#version 300 es" : "#version 150";
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

UiRenderer::~UiRenderer()
{}

std::shared_ptr<UiRenderer> ivfui::UiRenderer::create(GLFWwindow *window)
{
    return std::make_shared<UiRenderer>(window);
}

void UiRenderer::makeCurrent()
{
    if (m_context != nullptr)
        ImGui::SetCurrentContext(m_context);
#if IVF2_HAS_IMPLOT
    if (m_plotContext != nullptr)
        ImPlot::SetCurrentContext(static_cast<ImPlotContext *>(m_plotContext));
#endif
}

void UiRenderer::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UiRenderer::endFrame()
{
    ImGui::Render();
}

void UiRenderer::draw()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UiRenderer::shutdown()
{
    if (m_context == nullptr)
        return;

    // The window's GL context must be current (GLFWWindow::destroy ensures this).
    ImGui::SetCurrentContext(m_context);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
#if IVF2_HAS_IMPLOT
    if (m_plotContext != nullptr)
        ImPlot::DestroyContext(static_cast<ImPlotContext *>(m_plotContext));
    m_plotContext = nullptr;
#endif
    ImGui::DestroyContext(m_context);
    m_context = nullptr;
}

bool UiRenderer::wantCaptureMouse()
{
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool UiRenderer::wantCaptureKeyboard()
{
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

void ivfui::UiRenderer::scaleAllSizes(float scale)
{
    ImGui::GetStyle().ScaleAllSizes(scale);
}
