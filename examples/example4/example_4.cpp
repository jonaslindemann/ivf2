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

class ExampleWindow : public GLFWWindow {
private:
    CompositeNodePtr m_scene;
    CameraManipulatorPtr m_camManip;

    bool m_showDemoWindow = false;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        glEnable(GL_DEPTH_TEST);

        ShaderManagerPtr shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

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

        m_scene = CompositeNode::create();

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

        m_scene->add(box);
        m_scene->add(rbox);
        m_scene->add(sphere);
        m_scene->add(capCyl);
        m_scene->add(opCyl);
        m_scene->add(cone);
        m_scene->add(capCone);
        m_scene->add(dodeka);
        m_scene->add(capsule);
        m_scene->add(cappedTube);
        m_scene->add(tube);
        m_scene->add(disk);
        m_scene->add(plane);

        m_scene->add(axis);

        m_camManip = CameraManipulator::create(this->ref());

        return 0;
    }

    void onDraw()
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->draw();
    }

    void onUpdateOtherUi()
    {
        m_camManip->update();
    }

    void onResize(int width, int height)
    {
        m_camManip->update();
    }
};

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = ExampleWindow::create(800, 800, "Example 4");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
