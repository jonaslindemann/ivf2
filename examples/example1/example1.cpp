#include <iostream>
#include <memory>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <ivf/floatfield.h>
#include <ivf/vertexbuffer.h>
#include <ivf/vertexarray.h>
#include <ivf/vertices.h>
#include <ivf/colors.h>
#include <ivf/vertexshader.h>
#include <ivf/fragmentshader.h>
#include <ivf/program.h>
#include <ivf/shadermanager.h>
#include <ivf/transformmanager.h>
#include <ivf/axis.h>
#include <ivf/grid.h>
#include <ivf/utils.h>
#include <ivf/lightmanager.h>
#include <ivf/cube.h>
#include <ivf/cameramanipulator.h>
#include <ivf/normalfactory.h>
#include <ivf/material.h>
#include <ivf/compositenode.h>
#include <ivf/sphere.h>
#include <ivf/box.h>
#include <ivf/roundedbox.h>
#include <ivf/cappedcylinder.h>
#include <ivf/cylinder.h>
#include <ivf/cappedcone.h>
#include <ivf/cone.h>
#include <ivf/dodecahedron.h>

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

	auto window = glfwCreateWindow(800, 800, "OpenGL3", NULL, NULL);
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

	//NormalFactory nf;
	//std::shared_ptr<MeshNode> normalNode = nf.create(cubes[0]);

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