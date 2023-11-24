#include <iostream>
#include <memory>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
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

    auto shaderMgr = ShaderManager::create();
    shaderMgr->loadProgramFromFiles("shaders/basic.vert", "shaders/basic.frag", "basic");

    if (shaderMgr->compileLinkErrors())
    {
        cout << "Couldn't compile shaders, exiting..." << endl;
        return -1;
    }

    auto lightMgr = LightManager::create();

    auto pointLight1 = lightMgr->addPointLight();
    pointLight1->setEnabled(true);
    pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
    pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
    pointLight1->setAttenuation(1.0, 0.0, 0.0);
    pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));
    lightMgr->apply();

    auto scene = CompositeNode::create();

    auto axis = Axis::create();
    auto grid = Grid::create();

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

        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));
        material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

        node->setPos(glm::vec3(random(-20.0, 20.0), random(-20.0, 20.0), random(-20.0, 20.0)));
        node->setMaterial(material);

        scene->add(node);
    }

    // NormalFactory nf;
    // std::shared_ptr<MeshNode> normalNode = nf.create(cubes[0]);

    auto camManip = CameraManipulator::create(window);

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
