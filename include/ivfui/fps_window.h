#pragma once

#include <ivfui/ui_window.h>

namespace ivfui {

	class FpsWindow : public ivfui::UiWindow {
	private:
	public:
		FpsWindow();

		static std::shared_ptr<FpsWindow> create();

		void doDraw() override;

	};

	typedef std::shared_ptr<FpsWindow> FpsWindowPtr;
}



