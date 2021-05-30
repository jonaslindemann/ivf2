#include "examplewindow1.h"

using namespace ivf;

ExampleWindow1::ExampleWindow1()
		:UiWindow("ExampleWindow1"),
		m_counter(0),
		m_demoWindowButton(false),
		m_anotherWindowButton(false),
		m_clearColor{ 0.0f, 0.0f, 0.0f, 1.0f },
		m_f(0.0f)
{

}

std::shared_ptr<ExampleWindow1> ExampleWindow1::create()
{
	return std::make_shared<ExampleWindow1>();
}

void ExampleWindow1::doDraw()
{
	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

	if (ImGui::Button("Demo Window"))
		m_demoWindowButton = true;
	else
		m_demoWindowButton = false;

	if (ImGui::Button("Another Window"))
		m_anotherWindowButton = true;
	else
		m_anotherWindowButton = false;

	ImGui::SliderFloat("float", &m_f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&m_clearColor); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		m_counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", m_counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

int ExampleWindow1::counter()
{
	return m_counter;
}

bool ExampleWindow1::demoWindowButton()
{
	return m_demoWindowButton;
}

bool ExampleWindow1::anotherWindowButton()
{
	return m_anotherWindowButton;
}

float* ExampleWindow1::clearColor()
{
	return m_clearColor;
}