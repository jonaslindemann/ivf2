#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <map>

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

	auto window = glfwCreateWindow(800, 800, "Example 5", NULL, NULL);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	gladLoadGL();

	UiRendererPtr ui = UiRenderer::create(window);

	// During init, enable debug output

	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);

	// Font manager

	auto fontMgr = FontManager::create();
	fontMgr->loadFace("fonts/Gidole-Regular.ttf", "gidole");

	auto shaderMgr = ShaderManager::create();
	shaderMgr->loadProgramFromFiles("shaders/basic.vert", "shaders/basic.frag", "basic");

	if (shaderMgr->compileLinkErrors())
	{
		cout << "Couldn't compile shaders, exiting..." << endl;
		return -1;
	}

	// ---------------------------------------------------------------------------

	shaderMgr->currentProgram()->use();

	auto lightMgr = LightManager::create();
	lightMgr->enableLighting();

	auto dirLight = lightMgr->addDirectionalLight();
	dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
	dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
	dirLight->setEnabled(true);

	lightMgr->apply();

	auto material = Material::create();
	material->setDiffuseColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	material->setShininess(40.0);

	auto scene = CompositeNode::create();
	auto axis = Axis::create();


	auto text = TextNode::create();
	text->setText("Ivf++ 2.0");

	auto texture = Texture::create();
	texture->load("assets/planks.png");

	auto sphere = Sphere::create();
	sphere->setMaterial(material);
	sphere->setTexture(texture);


	/*
	for (auto i=0; i<11; i++)
		for(auto j=0; j<11; j++)
			for (auto k=0; k < 11; k++)
			{
				auto sphere = Sphere::create(0.15);
				sphere->setPos(glm::vec3(-5.0 + i, -5.0 + j, -5.0 + k));

				auto material = Material::create();
				material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0f));
				material->setShininess(40.0);

				sphere->setMaterial(material);
				scene->add(sphere);
			}
	*/

	scene->add(axis);
	scene->add(sphere);
	scene->add(text);

	auto fpsWindow = FpsWindow::create();

	auto camManip = CameraManipulator::create(window);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ui->beginFrame();

		fpsWindow->draw();

		if ((!ui->wantCaptureMouse()) && (!ui->wantCaptureKeyboard()))
			camManip->update();

		ui->endFrame();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		scene->draw();

		ui->draw();

		glfwSwapBuffers(window);
	}

	ui->shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}