#pragma once

#include <ivfui/ui_window.h>

class ExampleWindow2 : public ivfui::UiWindow {
private:
	bool m_closeMeButton;
public:
	ExampleWindow2();

	static std::shared_ptr<ExampleWindow2> create();

	void doDraw() override;

	bool closeMeButton();
};

typedef std::shared_ptr<ExampleWindow2> ExampleWindow2Ptr;

