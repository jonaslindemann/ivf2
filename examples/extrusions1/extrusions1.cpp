/**
 * @file extrusions1.cpp
 * @brief Extrusions example
 * @author Jonas Lindemann
 * @example extrusions1.cpp
 * @ingroup primitives_examples
 *
 * This example demonstrates the creation and rendering of extrusions
 * using solid lines and solid polylines. It sets up a basic scene with
 * an axis, grid, and two extrusion shapes: a solid line and a solid polyline.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

#include <glm/gtc/constants.hpp>

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

public:
    ExampleWindow(int width, int height, std::string title) : GLFWWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Enable depth testing for correct 3D rendering

        glEnable(GL_DEPTH_TEST);

        // Create and load font face for UI rendering

        auto fontMgr = FontManager::create();
        fontMgr->loadFace("fonts/Gidole-Regular.ttf", "gidole");

        // Create and load basic shader for rendering

        ShaderManagerPtr shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        // Check for shader compilation/linking errors

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Create and enable lighting manager

        auto lightMgr = LightManager::create();
        lightMgr->enableLighting();

        // Add and configure a directional light

        auto dirLight = lightMgr->addDirectionalLight();
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
        dirLight->setEnabled(true);

        // Apply lighting settings

        lightMgr->apply();

        // Create the scene graph root node

        m_scene = CompositeNode::create();

        // Add axis and grid to the scene

        AxisPtr axis = Axis::create();

        m_scene->add(axis);

        // Create yellow and red materials

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        auto greenMat = Material::create();
        greenMat->setDiffuseColor(glm::vec4(0.2, 0.9, 0.3, 1.0));

        auto blueMat = Material::create();
        blueMat->setDiffuseColor(glm::vec4(0.3, 0.5, 1.0, 1.0));

        // Create a solid yellow line and add to the scene

        auto line = SolidLine::create(glm::vec3(-6.0, -1.0, 0.0), glm::vec3(-6.0, 1.0, 0.0), 0.1);
        line->setMaterial(yellowMat);
        m_scene->add(line);

        // Solid polyline tube (now twist-free thanks to rotation-minimizing frames)

        auto poly = SolidPolyLine::create(0.1, 24);
        poly->addPoint(gml::dvec3(-4.5, -1.0, -1.0));
        poly->addPoint(gml::dvec3(-4.5, 1.0, 1.0));
        poly->addPoint(gml::dvec3(-4.5, 3.0, -1.0));
        poly->refresh();
        poly->setMaterial(yellowMat);
        m_scene->add(poly);

        // Solid polyline but using the Extrusion class (now twist-free thanks to rotation-minimizing frames)

        std::vector<glm::vec3> points;
        points.push_back(glm::vec3(-5.0, -1.0, -1.0));
        points.push_back(glm::vec3(-5.0, 1.0, 1.0));
        points.push_back(glm::vec3(-5.0, 3.0, -1.0));

        auto poly2 = Extrusion::create();
        poly2->setProfile(ExtrusionProfile::circle(0.15f, 24));
        poly2->setSpine(points, SpineInterp::Polyline, 3);
        poly2->setCaps(true, true);
        poly2->refresh();
        poly2->setMaterial(greenMat);
        m_scene->add(poly2);

        // Helix: a circle swept along a smooth (Catmull-Rom) spine. Demonstrates the absence of
        // twisting that the rotation-minimizing frames provide.

        std::vector<glm::vec3> helixPts;
        const int helixSamples = 48;
        for (int i = 0; i <= helixSamples; ++i)
        {
            float t = float(i) / float(helixSamples);
            float a = t * glm::two_pi<float>() * 2.0f;
            helixPts.push_back(glm::vec3(-2.5f + 0.8f * std::cos(a), -1.0f + 3.0f * t, 0.8f * std::sin(a)));
        }

        auto helix = Extrusion::create();
        helix->setProfile(ExtrusionProfile::circle(0.15f, 24));
        helix->setSpine(helixPts, SpineInterp::CatmullRom, 160);
        helix->setCaps(true, true);
        helix->refresh();
        helix->setMaterial(greenMat);
        m_scene->add(helix);

        // Rectangular profile swept along a wavy path with flat (facet) shading.

        std::vector<glm::vec3> wavePts;
        for (int i = 0; i <= 8; ++i)
        {
            float t = float(i) / 8.0f;
            wavePts.push_back(glm::vec3(-0.5f + t * 3.0f, std::sin(t * glm::two_pi<float>()), 0.0f));
        }

        auto ribbon = Extrusion::create();
        ribbon->setProfile(ExtrusionProfile::rectangle(0.5f, 0.15f));
        ribbon->setSpine(wavePts, SpineInterp::CatmullRom, 120);
        ribbon->setNormalStyle(NormalStyle::Facet);
        ribbon->setCaps(true, true);
        ribbon->refresh();
        ribbon->setMaterial(redMat);
        m_scene->add(ribbon);

        // Closed loop (ring): circle profile swept along a closed circular spine. The seam should
        // join without a twist/normal discontinuity.

        std::vector<glm::vec3> ringPts;
        const int ringCount = 32;
        for (int i = 0; i < ringCount; ++i)
        {
            float a = glm::two_pi<float>() * float(i) / float(ringCount);
            ringPts.push_back(glm::vec3(4.0f + 1.2f * std::cos(a), 0.0f, 1.2f * std::sin(a)));
        }

        auto ring = Extrusion::create();
        ring->setProfile(ExtrusionProfile::circle(0.18f, 20));
        ring->setSpine(ringPts, SpineInterp::Polyline);
        ring->setClosedSpine(true);
        ring->refresh();
        ring->setMaterial(blueMat);
        m_scene->add(ring);

        // Tapered + twisted extrusion via a per-station section transform.

        std::vector<glm::vec3> taperPts = {glm::vec3(7.0f, -1.5f, 0.0f), glm::vec3(7.0f, 1.5f, 0.0f)};
        auto taper = Extrusion::create();
        taper->setProfile(ExtrusionProfile::regularPolygon(5, 0.5f));
        taper->setSpine(taperPts, SpineInterp::Polyline);
        taper->setNormalStyle(NormalStyle::Facet);
        taper->setSectionTransform([](float t) {
            SectionTransform st;
            float s = 1.0f - 0.7f * t;
            st.scale = glm::vec2(s, s);
            st.twist = t * glm::two_pi<float>();
            return st;
        });
        taper->refresh();
        taper->setMaterial(greenMat);
        m_scene->add(taper);

        // Create camera manipulator for interactive view control

        m_camManip = CameraManipulator::create(this->ref());

        return 0;
    }

    void onDraw()
    {
        // Clear the screen and depth buffer

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the scene graph

        m_scene->draw();
    }

    void onUpdateOtherUi()
    {
        // Update the camera manipulator state

        m_camManip->update();
    }

    void onResize(int width, int height)
    {
        // Update the camera manipulator with new window dimensions

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

    auto window = ExampleWindow::create(1280, 800, "Extrusions");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
