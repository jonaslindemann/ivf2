#pragma once

#include <ivf/imguiwindow.h>

namespace ivf {

	class FpsWindow : public ivf::UiWindow {
	private:
	public:
		FpsWindow();

		static std::shared_ptr<FpsWindow> create();

		void doDraw() override;

	};

	typedef std::shared_ptr<FpsWindow> FpsWindowPtr;
}



