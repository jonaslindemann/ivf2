 #include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/ui.h>

#include "examplewindow1.h"
#include "examplewindow2.h"

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

	auto window = glfwCreateWindow(800, 800, "Example 2", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); 

	gladLoadGL();

	UiRendererPtr ui = UiRenderer::create(window);

	glfwGetWindowSize(window, &width, &height); 
	glViewport(0, 0, width, height);

	ShaderManagerPtr shaderMgr = ShaderManager::create();
	shaderMgr->loadProgramFromFiles("shaders/basic.vert", "shaders/basic.frag");
	
	if (shaderMgr->compileLinkErrors())
	{
		cout << "Couldn't compile shaders, exiting..." << endl;
		return -1;
	}

	LightManagerPtr lightMgr = LightManager::create();

	CompositeNodePtr scene = CompositeNode::create();

	AxisPtr axis = Axis::create();
	GridPtr grid = Grid::create();

	scene->add(axis);
	scene->add(grid);

	for (auto i = 0; i < 500; i++)
	{
		double r = random(0.0, 9.0);	

		TransformNodePtr node;

		if (r < 1.0)
			node = Cube::create();
		else if ((r >= 1) && (r < 2.0))
			node = Sphere::create(random(0.2, 1.0));
		else if ((r >= 2.0) && (r < 3.0))
			node = Box::create(glm::vec3(random(0.2, 2.0), random(0.2, 2.0), random(0.2, 2.0)));
		else if ((r >= 3.0) && (r < 4.0))
			node = RoundedBox::create(glm::vec3(random(0.2, 2.0), random(0.2, 2.0), random(0.2, 2.0)));
		else if ((r >= 4.0) && (r < 5.0))
			node = CappedCylinder::create(random(0.2, 1.0), random(0.2, 1.0));
		else if ((r >= 5.0) && (r < 6.0))
			node = Cylinder::create(random(0.2, 1.0), random(0.2, 1.0));
		else if ((r >= 6.0) && (r < 7.0))
			node = CappedCone::create(random(0.2, 1.0), random(0.2, 1.0));
		else if ((r >= 7.0) && (r < 8.0))
			node = Cone::create(random(0.2, 1.0), random(0.2, 1.0));
		else
			node = Dodecahedron::create(random(0.2, 1.0));

		MaterialPtr material = Material::create();
		material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));
		material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

		node->setPos(glm::vec3(random(-20.0, 20.0), random(-20.0, 20.0), random(-20.0, 20.0)));
		node->setMaterial(material);

		scene->add(node);
	}

	CameraManipulatorPtr camManip = CameraManipulator::create(window);

	static bool show_demo_window = false;

	ExampleWindow1Ptr exampleWindow1 = ExampleWindow1::create();
	ExampleWindow2Ptr exampleWindow2 = ExampleWindow2::create();

	exampleWindow2->hide();

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ui->beginFrame();

		exampleWindow1->draw();
		exampleWindow2->draw();
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		if (exampleWindow1->demoWindowButton())
			show_demo_window = true;

		if (exampleWindow1->anotherWindowButton())
			exampleWindow2->show();

		if (exampleWindow2->closeMeButton())
			exampleWindow2->hide();

		ui->endFrame();

		if ((!ui->wantCaptureMouse())&&(!ui->wantCaptureKeyboard()))
			camManip->update();

		scene->draw();

		ui->draw();

		glfwSwapBuffers(window);
	}

	ui->shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}