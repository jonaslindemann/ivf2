#pragma once

#include <memory>
#include <string>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace ivf {

	class UiWindow {
	private:
		std::string m_name;
		bool m_visible;
	public:
		UiWindow(const std::string name);
		virtual ~UiWindow();

		static std::shared_ptr<UiWindow> create(const std::string name);

		void draw();

		void setVisible(bool flag);
		bool visible();
		void show();
		void hide();

	protected:
		virtual void doDraw();
	};

	typedef std::shared_ptr<UiWindow> UiWindowPtr;

}


