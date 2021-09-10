#include <iostream>
#include <memory>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/ui.h>

using namespace ivf;
using namespace std;

int main()
{
	int width, height;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	auto window = glfwCreateWindow(800, 800, "Example 1", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	gladLoadGL();

	glfwGetWindowSize(window, &width, &height); 
	glViewport(0, 0, width, height);

	ShaderManagerPtr shaderMgr = ShaderManager::create();
	shaderMgr->loadProgramFromFiles("shaders/basic.vert", "shaders/basic.frag", "basic");
	
	if (shaderMgr->compileLinkErrors())
	{
		cout << "Couldn't compile shaders, exiting..." << endl;
		return -1;
	}

	LightManagerPtr lightMgr = LightManager::create();

	auto pointLight1 = lightMgr->addPointLight();
	pointLight1->setEnabled(true);
	pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
	pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
	pointLight1->setAttenuation(1.0, 0.0, 0.0);
	pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));
	lightMgr->apply();

	CompositeNodePtr scene = CompositeNode::create();

	AxisPtr axis = Axis::create();
	GridPtr grid = Grid::create();

	scene->add(axis);
	scene->add(grid);

	ExtrusionPtr extrusion = Extrusion::create();

	glm::vec3 p(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 20; i++)
	{
		extrusion->addPathPoint(p);
		p += glm::vec3(random(-1.0, 1.0), random(-1.0, 1.0), random(-1.0, 1.0));
	}

	extrusion->addSectionPoint(glm::vec2(-0.1, -0.1));
	extrusion->addSectionPoint(glm::vec2( 0.1, -0.1));
	extrusion->addSectionPoint(glm::vec2( 0.1,  0.1));
	extrusion->addSectionPoint(glm::vec2(-0.1,  0.1));

	extrusion->refresh();

	scene->add(extrusion);

	CameraManipulatorPtr camManip = CameraManipulator::create(window);

	glEnable(GL_DEPTH_TEST);

	double previousTime = glfwGetTime();
	int frameCount = 0;

	while (!glfwWindowShouldClose(window)) 
	{
		// Measure speed

		double currentTime = glfwGetTime();
		frameCount++;

		// If a second has passed.

		if (currentTime - previousTime >= 1.0)
		{
			// Display the frame count here any way you want.

			std::cout << frameCount << endl;
			frameCount = 0;
			previousTime = currentTime;
		}

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camManip->update();

		scene->draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}