#include "examplewindow2.h"

using namespace ivf;

ExampleWindow2::ExampleWindow2()
	:UiWindow("ExampleWindow2"),
	 m_closeMeButton(false)
{
}

std::shared_ptr<ExampleWindow2> ExampleWindow2::create()
{
	return std::make_shared<ExampleWindow2>();
}

void ExampleWindow2::doDraw()
{
	ImGui::Text("Hello ExampleWindow2");

	if (ImGui::Button("Close Me"))
		m_closeMeButton = true;
	else
		m_closeMeButton = false;
}


bool ExampleWindow2::closeMeButton()
{
	bool state = m_closeMeButton;
	m_closeMeButton = false;
	return state;
}