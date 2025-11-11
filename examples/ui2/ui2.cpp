/**
 * @file ui1.cpp
 * @brief UI example with multiple transforms
 * @author Jonas Lindemann
 * @example ui1.cpp
 * @ingroup ui_examples
 *
 * This example demonstrates the use of the IVF library to create a scene with
 * various geometric shapes, a camera manipulator, and multiple UI windows.
 * It includes a main window with a scene containing randomly positioned and colored shapes,
 * two example windows for additional UI functionality,
 * and a camera manipulator for navigating the scene.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

    CubePtr m_cube;
    SpherePtr m_sphere;
    CappedCylinderPtr m_cylinder;
    ConePtr m_cone;
    CappedTubePtr m_cappedTube;

    MaterialPtr m_redMat;
    MaterialPtr m_greenMat;
    MaterialPtr m_blueMat;
    MaterialPtr m_yellowMat;
    MaterialPtr m_whiteMat;

    double m_radius;      ///< Outer radius of the tube.
    double m_innerRadius; ///< Inner radius of the tube (hole radius).
    double m_size;        ///< Height of the tube.
    double m_start;       ///< Start angle (in radians) for the sweep.
    double m_sweep;       ///< Sweep angle (in radians) for the tube.

    double m_minHeight = 0.1;
    double m_maxHeight = 5.0;

    bool m_window1 = false;
    bool m_window2 = false;
    bool m_window3 = false;
    bool m_window4 = false;
    bool m_window5 = false;

    float m_cubePos[3] = {-3.0f, 0.0f, 0.0f};
    float m_spherePos[3] = {3.0f, 0.0f, 0.0f};
    float m_cylinderPos[3] = {0.0f, 0.0f, -3.0f};
    float m_conePos[3] = {0.0f, 0.0f, 3.0f};

    float m_cubeColor[3] = {1.0f, 0.0f, 0.0f};
    float m_sphereColor[3] = {0.0f, 1.0f, 0.0f};
    float m_cylinderColor[3] = {0.0f, 0.0f, 1.0f};
    float m_coneColor[3] = {1.0f, 1.0f, 0.0f};
    float m_cappedTubeColor[3] = {1.0f, 1.0f, 1.0f};

public:
    ExampleWindow(int width, int height, std::string title) : GLFWWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Create and initialize the shader manager, then load the basic shader.

        auto shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        // Check for shader compilation or linking errors.

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Create the light manager.

        auto lightMgr = LightManager::create();

        // Add and configure a point light.

        auto pointLight1 = lightMgr->addPointLight();
        pointLight1->setEnabled(true);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.0, 0.0);
        pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));

        lightMgr->apply();

        // Create the main scene composite node.

        m_scene = CompositeNode::create();

        // Add an axis to the scene.

        auto axis = Axis::create();

        m_scene->add(axis);

        m_redMat = Material::create();
        m_redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
        m_redMat->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_redMat->setShininess(32.0f);

        m_greenMat = Material::create();
        m_greenMat->setDiffuseColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
        m_greenMat->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_greenMat->setShininess(32.0f);

        m_blueMat = Material::create();
        m_blueMat->setDiffuseColor(glm::vec4(0.0, 0.0, 1.0, 1.0));
        m_blueMat->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_blueMat->setShininess(32.0f);

        m_yellowMat = Material::create();
        m_yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
        m_yellowMat->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_yellowMat->setShininess(32.0f);

        m_whiteMat = Material::create();
        m_whiteMat->setDiffuseColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_whiteMat->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_whiteMat->setShininess(32.0f);

        m_cube = Cube::create();
        m_cube->setPos(glm::vec3(-3.0, 0.0, 0.0));
        m_cube->setMaterial(m_redMat);

        m_sphere = Sphere::create(0.5);
        m_sphere->setPos(glm::vec3(3.0, 0.0, 0.0));
        m_sphere->setMaterial(m_greenMat);

        m_cylinder = CappedCylinder::create(0.5, 1.0);
        m_cylinder->setPos(glm::vec3(0.0, 0.0, -3.0));
        m_cylinder->setMaterial(m_blueMat);

        m_cone = Cone::create(0.5, 0.5);
        m_cone->setPos(glm::vec3(0.0, 0.0, 3.0));
        m_cone->setMaterial(m_yellowMat);

        m_cappedTube = CappedTube::create(0.5, 0.3, 1.0, 32, 8, 1, 0.0, glm::radians(360.0));
        m_cappedTube->setPos(glm::vec3(0.0, 0.0, 0.0));
        m_cappedTube->setMaterial(m_whiteMat);

        m_radius = m_cappedTube->radius();
        m_innerRadius = m_cappedTube->innerRadius();
        m_size = m_cappedTube->height();
        m_start = m_cappedTube->startAngle();
        m_sweep = m_cappedTube->sweepAngle();

        m_scene->add(m_cube);
        m_scene->add(m_sphere);
        m_scene->add(m_cylinder);
        m_scene->add(m_cone);
        m_scene->add(m_cappedTube);

        // Setup variables for dodecahedron controls

        // Create the camera manipulator for scene navigation.

        m_camManip = CameraManipulator::create(this->ref());
        m_camManip->setCameraPosition(glm::vec3(0.0, 5.0, 10.0));

        // Setup ImGui

        this->scaleAllSizes(2.0f);

        return 0;
    }

    void onDraw()
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->draw();
    }

    void onUpdate()
    {}

    void onUpdateOtherUi()
    {
        m_camManip->update();
    }

    void onDrawUi()
    {
        ImGui::Begin("A first window", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Hello from window1!");
        ImGui::Text("Use the number 1-5 to open the example windows.");
        ImGui::End();

        ImGui::Begin("A second window", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Frame time %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();

        if (m_window1)
        {
            ImGui::Begin("A third window", &m_window1, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Hello from window1!");
            ImGui::End();
        }

        if (m_window2)
        {
            ImGui::Begin("A similar window", &m_window2, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Hello from window1!");

            if (ImGui::Button("Close Me"))
                m_window2 = false;

            ImGui::End();
        }

        if (m_window3)
        {
            ImGui::Begin("CappedTube controls", &m_window3, ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::InputScalar("Radius", ImGuiDataType_Double, &m_radius))
                m_cappedTube->setRadius(m_radius);

            if (ImGui::DragScalar("Inner Radius", ImGuiDataType_Double, &m_innerRadius))
                m_cappedTube->setInnerRadius(m_innerRadius);

            if (ImGui::SliderScalar("Height", ImGuiDataType_Double, &m_size, &m_minHeight, &m_maxHeight))
                m_cappedTube->setHeight(m_size);

            if (ImGui::DragScalar("Start Angle", ImGuiDataType_Double, &m_start))
                m_cappedTube->setStartAngle(m_start);

            if (ImGui::DragScalar("Sweep Angle", ImGuiDataType_Double, &m_sweep))
                m_cappedTube->setSweepAngle(m_sweep);

            ImGui::End();
        }

        if (m_window4)
        {
            ImGui::Begin("Another window", &m_window4, ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::SeparatorText("Transform Controls");

            if (ImGui::SliderFloat3("Cube Position", m_cubePos, -10.0f, 10.0f))
                m_cube->setPos(glm::vec3(m_cubePos[0], m_cubePos[1], m_cubePos[2]));

            if (ImGui::SliderFloat3("Sphere Position", m_spherePos, -10.0f, 10.0f))
                m_sphere->setPos(glm::vec3(m_spherePos[0], m_spherePos[1], m_spherePos[2]));

            if (ImGui::SliderFloat3("Cylinder Position", m_cylinderPos, -10.0f, 10.0f))
                m_cylinder->setPos(glm::vec3(m_cylinderPos[0], m_cylinderPos[1], m_cylinderPos[2]));

            if (ImGui::SliderFloat3("Cone Position", m_conePos, -10.0f, 10.0f))
                m_cone->setPos(glm::vec3(m_conePos[0], m_conePos[1], m_conePos[2]));

            ImGui::SeparatorText("Color Controls");

            if (ImGui::ColorEdit3("Cube Color", m_cubeColor))
            {
                auto mat = m_cube->material();
                mat->setDiffuseColor(glm::vec4(m_cubeColor[0], m_cubeColor[1], m_cubeColor[2], 1.0f));
                m_cube->setMaterial(mat);
            }

            if (ImGui::ColorEdit3("Sphere Color", m_sphereColor))
            {
                auto mat = m_sphere->material();
                mat->setDiffuseColor(glm::vec4(m_sphereColor[0], m_sphereColor[1], m_sphereColor[2], 1.0f));
                m_sphere->setMaterial(mat);
            }

            if (ImGui::ColorEdit3("Cylinder Color", m_cylinderColor))
            {
                auto mat = m_cylinder->material();
                mat->setDiffuseColor(glm::vec4(m_cylinderColor[0], m_cylinderColor[1], m_cylinderColor[2], 1.0f));
                m_cylinder->setMaterial(mat);
            }

            if (ImGui::ColorEdit3("Cone Color", m_coneColor))
            {
                auto mat = m_cone->material();
                mat->setDiffuseColor(glm::vec4(m_coneColor[0], m_coneColor[1], m_coneColor[2], 1.0f));
                m_cone->setMaterial(mat);
            }

            if (ImGui::ColorEdit3("CappedTube Color", m_cappedTubeColor))
            {
                auto mat = m_cappedTube->material();
                mat->setDiffuseColor(glm::vec4(m_cappedTubeColor[0], m_cappedTubeColor[1], m_cappedTubeColor[2], 1.0f));
                m_cappedTube->setMaterial(mat);
            }

            ImGui::SeparatorText("Visibility Controls");

            if (m_cube->visible())
            {
                if (ImGui::Button("Hide Cube"))
                    m_cube->setVisible(false);
            }
            else
            {
                if (ImGui::Button("Show Cube"))
                    m_cube->setVisible(true);
            }

            ImGui::SameLine();

            if (m_sphere->visible())
            {
                if (ImGui::Button("Hide Sphere"))
                    m_sphere->setVisible(false);
            }
            else
            {
                if (ImGui::Button("Show Sphere"))
                    m_sphere->setVisible(true);
            }

            ImGui::SameLine();

            if (m_cylinder->visible())
            {
                if (ImGui::Button("Hide Cylinder"))
                    m_cylinder->setVisible(false);
            }
            else
            {
                if (ImGui::Button("Show Cylinder"))
                    m_cylinder->setVisible(true);
            }

            ImGui::SameLine();

            if (m_cone->visible())
            {
                if (ImGui::Button("Hide Cone"))
                    m_cone->setVisible(false);
            }
            else
            {
                if (ImGui::Button("Show Cone"))
                    m_cone->setVisible(true);
            }

            ImGui::End();
        }

        if (m_window5)
        {
            ImGui::Begin("Yet another window", &m_window5, ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::BeginTabBar("MyTabBar"))
            {
                if (ImGui::BeginTabItem("Transform Controls"))
                {
                    if (ImGui::SliderFloat3("Cube Position", m_cubePos, -10.0f, 10.0f))
                        m_cube->setPos(glm::vec3(m_cubePos[0], m_cubePos[1], m_cubePos[2]));
                    if (ImGui::SliderFloat3("Sphere Position", m_spherePos, -10.0f, 10.0f))
                        m_sphere->setPos(glm::vec3(m_spherePos[0], m_spherePos[1], m_spherePos[2]));
                    if (ImGui::SliderFloat3("Cylinder Position", m_cylinderPos, -10.0f, 10.0f))
                        m_cylinder->setPos(glm::vec3(m_cylinderPos[0], m_cylinderPos[1], m_cylinderPos[2]));
                    if (ImGui::SliderFloat3("Cone Position", m_conePos, -10.0f, 10.0f))
                        m_cone->setPos(glm::vec3(m_conePos[0], m_conePos[1], m_conePos[2]));
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Color Controls"))
                {
                    if (ImGui::ColorEdit3("Cube Color", m_cubeColor))
                    {
                        auto mat = m_cube->material();
                        mat->setDiffuseColor(glm::vec4(m_cubeColor[0], m_cubeColor[1], m_cubeColor[2], 1.0f));
                        m_cube->setMaterial(mat);
                    }
                    if (ImGui::ColorEdit3("Sphere Color", m_sphereColor))
                    {
                        auto mat = m_sphere->material();
                        mat->setDiffuseColor(glm::vec4(m_sphereColor[0], m_sphereColor[1], m_sphereColor[2], 1.0f));
                        m_sphere->setMaterial(mat);
                    }
                    if (ImGui::ColorEdit3("Cylinder Color", m_cylinderColor))
                    {
                        auto mat = m_cylinder->material();
                        mat->setDiffuseColor(
                            glm::vec4(m_cylinderColor[0], m_cylinderColor[1], m_cylinderColor[2], 1.0f));
                    }
                    if (ImGui::ColorEdit3("Cone Color", m_coneColor))
                    {
                        auto mat = m_cone->material();
                        mat->setDiffuseColor(glm::vec4(m_coneColor[0], m_coneColor[1], m_coneColor[2], 1.0f));
                        m_cone->setMaterial(mat);
                    }
                    if (ImGui::ColorEdit3("CappedTube Color", m_cappedTubeColor))
                    {
                        auto mat = m_cappedTube->material();
                        mat->setDiffuseColor(
                            glm::vec4(m_cappedTubeColor[0], m_cappedTubeColor[1], m_cappedTubeColor[2], 1.0f));
                        m_cappedTube->setMaterial(mat);
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Visibility Controls"))
                {
                    if (m_cube->visible())
                    {
                        if (ImGui::Button("Hide Cube"))
                            m_cube->setVisible(false);
                    }
                    else
                    {
                        if (ImGui::Button("Show Cube"))
                            m_cube->setVisible(true);
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();
        }
    }

    void onKey(int key, int scancode, int action, int mods)
    {
        if ((key == GLFW_KEY_1) && (action == GLFW_PRESS))
            m_window1 = !m_window1;

        if ((key == GLFW_KEY_2) && (action == GLFW_PRESS))
            m_window2 = !m_window2;

        if ((key == GLFW_KEY_3) && (action == GLFW_PRESS))
            m_window3 = !m_window3; 

        if ((key == GLFW_KEY_4) && (action == GLFW_PRESS))
            m_window4 = !m_window4;

        if ((key == GLFW_KEY_5) && (action == GLFW_PRESS))
            m_window5 = !m_window5;
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

    auto window = ExampleWindow::create(800, 800, "User interface");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
