#include <ivfui/fps_window.h>

#include <ivf/light_manager.h>
#include <ivfui/glfw_application.h>

using namespace ivfui;
using namespace ivf;

FpsWindow::FpsWindow()
	:UiWindow("FPS")
{
}

std::shared_ptr<FpsWindow> FpsWindow::create()
{
	return std::make_shared<FpsWindow>();
}

void FpsWindow::doDraw()
{
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// The CPU loop can keep up with the display while the GPU falls behind,
	// so show the GPU time separately
	auto window = GLFWWindowTracker::instance()->get(glfwGetCurrentContext());
	if (window && window->gpuFrameTime() > 0.0)
		ImGui::Text("GPU %.3f ms/frame (max %.1f FPS)", window->gpuFrameTime() * 1000.0, 1.0 / window->gpuFrameTime());
}

