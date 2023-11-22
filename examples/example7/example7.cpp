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

    auto yellowMat = Material::create();
    yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

    auto redMat = Material::create();
    redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

    auto greenMat = Material::create();
    greenMat->setDiffuseColor(glm::vec4(0.0, 1.0, 0.0, 1.0));

    auto line = SolidLine::create(glm::vec3(0.0, -1.0, -1.0), glm::vec3(0.0, 1.0, 1.0), 0.1);
    line->setMaterial(yellowMat);

    auto extrusion = Extrusion::create();

    extrusion->createCircleSection(0.1);

    glm::vec3 pos;

    float angle = 0.0f;
    constexpr float da = glm::pi<float>() / 36.0f;

    while (angle < glm::quarter_pi<float>())
    {
        pos = glm::vec3(2.0 * cos(angle), 0.0, 2.0 * sin(angle));
        extrusion->addPathPoint(pos);
        angle += da;
    }
    // pos = glm::vec3(2.0 * cos(0.0f), 0.0, 2.0 * sin(0.0f));
    // extrusion->addPathPoint(pos);

    extrusion->refresh();

    extrusion->setMaterial(yellowMat);

    auto sphere1 = Sphere::create();
    sphere1->setPos(glm::vec3(-8.0, 0.0, 0.0));
    sphere1->setMaterial(redMat);

    auto sphere2 = Sphere::create();
    sphere2->setPos(glm::vec3(8.0, 0.0, 0.0));
    sphere2->setMaterial(greenMat);

    scene->add(sphere1);
    scene->add(sphere2);
    scene->add(line);
    scene->add(extrusion);

    CameraManipulatorPtr camManip = CameraManipulator::create(window);

    glEnable(GL_DEPTH_TEST);

    double previousTime = glfwGetTime();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        // If a second has passed.

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
