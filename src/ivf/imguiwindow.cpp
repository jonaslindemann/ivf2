#include <ivf/imguiwindow.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace ivf;

UiWindow::UiWindow(const std::string name)
	:m_name(name),
	 m_visible(true)
{
}

UiWindow::~UiWindow()
{
}

std::shared_ptr<UiWindow> ivf::UiWindow::create(const std::string name)
{
	return std::make_shared<UiWindow>(name);
}

void ivf::UiWindow::draw()
{
	if (m_visible)
	{
		ImGui::Begin(m_name.c_str(), 0, ImGuiWindowFlags_AlwaysAutoResize); //, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		doDraw();
		ImGui::End();
	}
}

void ivf::UiWindow::setVisible(bool flag)
{
	m_visible = flag;
}

bool ivf::UiWindow::visible()
{
	return m_visible;
}

void ivf::UiWindow::show()
{
	m_visible = true;
}

void ivf::UiWindow::hide()
{
	m_visible = false;
}

void ivf::UiWindow::doDraw()
{
}
