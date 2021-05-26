#pragma once

#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

namespace ivf {

	class CameraManipulator {
	private:
		GLFWwindow* m_window;
		int m_width;
		int m_height;
		bool m_firstClick;
		bool m_firstTime;
		double m_mouseStartX;
		double m_mouseStartY;
		double m_mouseX;
		double m_mouseY;
		glm::vec3 m_cameraTarget;
		glm::vec3 m_cameraPosition;
		glm::vec3 m_cameraNewPos;
		glm::vec3 m_cameraNewTarget;

	public:
		CameraManipulator(GLFWwindow* window);

		static std::shared_ptr<CameraManipulator> create(GLFWwindow* window);

		void update();
	};

	typedef std::shared_ptr<CameraManipulator> CameraManipulatorPtr;

}