#pragma once

#include <imgui.h>

#include <GLFW/glfw3.h>

#include <memory>

namespace ivfui {

	class UiRenderer {
	private:

	public:
		UiRenderer(GLFWwindow* window);
		virtual ~UiRenderer();

		static std::shared_ptr<UiRenderer> create(GLFWwindow* window);

		void beginFrame();
		void endFrame();

		void draw();

		void shutdown();

		bool wantCaptureMouse();
		bool wantCaptureKeyboard();
	};

	typedef std::shared_ptr<UiRenderer> UiRendererPtr;

}


