#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

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

    auto window = glfwCreateWindow(800, 800, "Example 4", NULL, NULL);
    if (window == NULL) {
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

    auto ui = UiRenderer::create(window);

    auto shaderMgr = ShaderManager::create();
    shaderMgr->loadBasicShader();

    if (shaderMgr->compileLinkErrors()) {
        cout << "Couldn't compile shaders, exiting..." << endl;
        return -1;
    }

    auto lightMgr = LightManager::create();
    auto pointLight1 = lightMgr->addPointLight();
    pointLight1->setEnabled(true);
    pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
    pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
    pointLight1->setAttenuation(1.0, 0.0, 0.0);
    pointLight1->setPosition(glm::vec3(10.0, 10.0, 10.0));
    lightMgr->apply();

    auto scene = CompositeNode::create();

    auto axis = Axis::create();

    auto material = Material::create();
    material->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    material->setUseTexture(true);
    material->setUseTexture(false);
    material->setShininess(50.0);

    auto sphere = Sphere::create();
    sphere->setMaterial(material);
    sphere->setPos(glm::vec3(-3.0, 0.0, -3.0));

    auto box = Box::create();
    box->setMaterial(material);
    box->setPos(glm::vec3(0.0, .0, -3.0));

    auto rbox = RoundedBox::create();
    rbox->setMaterial(material);
    rbox->setPos(glm::vec3(3.0, 0.0, -3.0));

    auto capCyl = CappedCylinder::create();
    capCyl->setMaterial(material);
    capCyl->setPos(glm::vec3(-3.0, 0.0, 0.0));

    auto opCyl = Cylinder::create();
    opCyl->setMaterial(material);
    opCyl->setPos(glm::vec3(0.0, 0.0, 0.0));

    auto cone = Cone::create();
    cone->setMaterial(material);
    cone->setPos(glm::vec3(3.0, 0.0, 0.0));

    auto capCone = CappedCone::create();
    capCone->setMaterial(material);
    capCone->setPos(glm::vec3(-3.0, 0.0, 3.0));

    auto dodeka = Dodecahedron::create();
    dodeka->setMaterial(material);
    dodeka->setPos(glm::vec3(0.0, 0.0, 3.0));

    auto capsule = Capsule::create();
    capsule->setMaterial(material);
    capsule->setPos(glm::vec3(3.0, 0.0, 3.0));

    auto cappedTube = CappedTube::create();
    cappedTube->setMaterial(material);
    cappedTube->setPos(glm::vec3(-3.0, 0.0, 6.0));

    auto tube = Tube::create();
    tube->setMaterial(material);
    tube->setPos(glm::vec3(0.0, 0.0, 6.0));

    auto disk = Disk::create();
    disk->setMaterial(material);
    disk->setPos(glm::vec3(3.0, 0.0, 6.0));

    auto plane = Plane::create();
    plane->setMaterial(material);
    plane->setPos(glm::vec3(-3.0, 0.0, 9.0));

    scene->add(box);
    scene->add(rbox);
    scene->add(sphere);
    scene->add(capCyl);
    scene->add(opCyl);
    scene->add(cone);
    scene->add(capCone);
    scene->add(dodeka);
    scene->add(capsule);
    scene->add(cappedTube);
    scene->add(tube);
    scene->add(disk);
    scene->add(plane);

    scene->add(axis);

    auto camManip = CameraManipulator::create(window);

    while (!glfwWindowShouldClose(window)) {
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
