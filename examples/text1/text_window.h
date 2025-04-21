#pragma once

#include <ivfui/ui_window.h>
#include <ivf/text_node.h>

#include <array>

class TextWindow : public ivfui::UiWindow {
private:
	std::string m_text;
	std::array<char, 255> m_textArray;
	ivf::TextNodePtr m_textNode;
	float m_size;
	int m_alignX;
	int m_alignY;
public:
	TextWindow(ivf::TextNodePtr textNode);

	static std::shared_ptr<TextWindow> create(ivf::TextNodePtr textNode);

	void doDraw() override;
};

typedef std::shared_ptr<TextWindow> TextWindowPtr;

