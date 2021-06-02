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

	auto window = glfwCreateWindow(800, 800, "Example 4", NULL, NULL);
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

	MaterialPtr material = Material::create();
	material->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	material->setUseTexture(true);
	material->setUseTexture(false);

	SpherePtr sphere = Sphere::create();
	sphere->setMaterial(material);
	sphere->setPos(glm::vec3(-3.0, 0.0, -3.0));

	BoxPtr box = Box::create();
	box->setMaterial(material);
	box->setPos(glm::vec3(0.0, .0, -3.0));

	RoundedBoxPtr rbox = RoundedBox::create();
	rbox->setMaterial(material);
	rbox->setPos(glm::vec3(3.0, 0.0, -3.0));

	CappedCylinderPtr capCyl = CappedCylinder::create();
	capCyl->setMaterial(material);
	capCyl->setPos(glm::vec3(-3.0, 0.0, 0.0));

	CylinderPtr opCyl = Cylinder::create();
	opCyl->setMaterial(material);
	opCyl->setPos(glm::vec3(0.0, 0.0, 0.0));

	ConePtr cone = Cone::create();
	cone->setMaterial(material);
	cone->setPos(glm::vec3(3.0, 0.0, 0.0));

	CappedConePtr capCone = CappedCone::create();
	capCone->setMaterial(material);
	capCone->setPos(glm::vec3(-3.0, 0.0, 3.0));

	DodecahedronPtr dodeka = Dodecahedron::create();
	dodeka->setMaterial(material);
	dodeka->setPos(glm::vec3(0.0, 0.0, 3.0));

	CapsulePtr capsule = Capsule::create();
	capsule->setMaterial(material);
	capsule->setPos(glm::vec3(3.0, 0.0, 3.0));

	scene->add(box);
	scene->add(rbox);
	scene->add(sphere);
	scene->add(capCyl);
	scene->add(opCyl);
	scene->add(cone);
	scene->add(capCone);
	scene->add(dodeka);
	scene->add(capsule);

	scene->add(axis);

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