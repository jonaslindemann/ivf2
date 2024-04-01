#include <ivfui/ui_manager.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ivfui/ui_font.h>

using namespace ivfui;

UiRenderer::UiRenderer(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

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

    const char *glsl_version = "#version 150";
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

UiRenderer::~UiRenderer()
{}

std::shared_ptr<UiRenderer> ivfui::UiRenderer::create(GLFWwindow *window)
{
    return std::make_shared<UiRenderer>(window);
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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
