#include "textwindow.h"

using namespace ivf;

TextWindow::TextWindow(TextNodePtr node)
		:UiWindow("TextWindow"),
		m_text(""),
		m_textArray(),
		m_textNode(node),
		m_size(1.0f), 
		m_alignX(int(node->alignX())),
		m_alignY(int(node->alignY()))
{
	m_textArray.assign(0);
	m_text = m_textNode->text();
	std::copy(m_text.begin(), m_text.end(), m_textArray.data());
}

std::shared_ptr<TextWindow> TextWindow::create(ivf::TextNodePtr textNode)
{
	return std::make_shared<TextWindow>(textNode);
}

void TextWindow::doDraw()
{
	if (ImGui::InputText("string", m_textArray.data(), 255))
	{
		m_text = m_textArray.data();
		m_textNode->setText(m_text);
	}

	if (ImGui::SliderFloat("Size", &m_size, 1.0f, 20.0f))
	{
		m_textNode->setSize(m_size);
	}

	if (ImGui::RadioButton("Left", &m_alignX, int(ivf::TextAlignX::LEFT)))
		m_textNode->setAlignX(ivf::TextAlignX::LEFT);

	ImGui::SameLine();
	if (ImGui::RadioButton("Center", &m_alignX, int(ivf::TextAlignX::CENTER)))
		m_textNode->setAlignX(ivf::TextAlignX::CENTER);

	ImGui::SameLine();	
	if (ImGui::RadioButton("Right", &m_alignX, int(ivf::TextAlignX::RIGHT)))
		m_textNode->setAlignX(ivf::TextAlignX::RIGHT);

	if (ImGui::RadioButton("Bottom", &m_alignY, int(ivf::TextAlignY::BOTTOM)))
		m_textNode->setAlignY(ivf::TextAlignY::BOTTOM);

	ImGui::SameLine();
	if (ImGui::RadioButton("Middle", &m_alignY, int(ivf::TextAlignY::MIDDLE)))
		m_textNode->setAlignY(ivf::TextAlignY::MIDDLE);
	
	ImGui::SameLine();
	if (ImGui::RadioButton("Top", &m_alignY, int(ivf::TextAlignY::TOP)))
		m_textNode->setAlignY(ivf::TextAlignY::TOP);
}