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

	auto window = glfwCreateWindow(800, 800, "Example 3", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); 

	gladLoadGL();

	// During init, enable debug output

	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);

	UiRendererPtr ui = UiRenderer::create(window);

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

	MaterialPtr sphereMaterial = Material::create();
	sphereMaterial->setDiffuseColor(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
	sphereMaterial->setUseTexture(true);
	sphereMaterial->setUseTexture(false);

	TexturePtr textureCat = Texture::create();
	textureCat->load("assets/pop_cat.png");

 	TexturePtr textureBrick = Texture::create();
	textureBrick->load("assets/brick.png");

	SpherePtr sphere = Sphere::create();
	sphere->setTexture(textureCat);
	sphere->setMaterial(sphereMaterial);
	sphere->setPos(glm::vec3(0.0, 3.0, 0.0));

	BoxPtr box = Box::create();
	box->setTexture(textureBrick);
	box->setMaterial(sphereMaterial);
	box->setPos(glm::vec3(3.0, 0.0, 0.0));

	scene->add(box);
	scene->add(sphere);

	scene->add(axis);
	scene->add(grid);

	CameraManipulatorPtr camManip = CameraManipulator::create(window);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene->draw();

		ui->beginFrame();

		if ((!ui->wantCaptureMouse()) && (!ui->wantCaptureKeyboard()))
			camManip->update();

		ui->endFrame();

		ui->draw();

		glfwSwapBuffers(window);
	}

	ui->shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}