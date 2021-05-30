#pragma once

#include <ivf/imguiwindow.h>

class ExampleWindow2 : public ivf::UiWindow {
private:
	bool m_closeMeButton;
public:
	ExampleWindow2();

	static std::shared_ptr<ExampleWindow2> create();

	void doDraw() override;

	bool closeMeButton();
};

typedef std::shared_ptr<ExampleWindow2> ExampleWindow2Ptr;

