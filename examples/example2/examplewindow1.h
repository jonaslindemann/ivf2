#pragma once

#include <ivf/imguiwindow.h>

class ExampleWindow1 : public ivf::UiWindow {
private:
	int m_counter;
	bool m_demoWindowButton;
	bool m_anotherWindowButton;
	float m_clearColor[4];
	float m_f;
public:
	ExampleWindow1();

	static std::shared_ptr<ExampleWindow1> create();

	void doDraw() override;

	int counter();

	bool demoWindowButton();
	bool anotherWindowButton();

	float* clearColor();
};

typedef std::shared_ptr<ExampleWindow1> ExampleWindow1Ptr;

