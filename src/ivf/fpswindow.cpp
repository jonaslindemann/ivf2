#include <ivf/fpswindow.h>

#include <ivf/lightmanager.h>

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
}

