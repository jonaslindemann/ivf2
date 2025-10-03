/**
 * @file assimp1.cpp
 * @brief Assimp model loading example
 * @author Jonas Lindemann
 * @example assimp1.cpp
 * @ingroup file_io_examples
 *
 * This example demonstrates loading and displaying a 3D model using the Assimp library.
 * It creates a scene with a grid, axis, and a model loaded from an .ac file.
 */

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfui/scene_inspector.h>

#include <ivf/node_visitor.h>
#include <ivf/stock_shaders.h>
#include <ivf/blur_effect.h>
#include <ivf/spline_animation.h>
#include <ivf/keyframe_animation.h>
#include <ivf/model_loader.h>

#include <ivfmath/spline.h>

#include <glm/gtx/intersect.hpp>

using namespace ivf;
using namespace ivfui;
using namespace ivfmath;

using namespace std;

class EditorWindow : public GLFWSceneWindow {
private:
    bool m_openFileDialog{false};

    SceneInspectorPtr m_sceneInspector{nullptr};

public:
    EditorWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<EditorWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<EditorWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        // Enable a headlight for basic scene illumination

        this->enableHeadlight();

        // Create and add an axis to the scene for orientation reference

        this->setAxisVisible(true);

        this->grid()->setPlane(GridPlane::XZ); // Restrict grid to the XY plane (Z=0) for a floor-like appearance
        this->setGridVisible(true);

        // Setup camera

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0f, 0.0f, 10.0f));
        this->cameraManipulator()->setFarZ(1000.0f);
        this->cameraManipulator()->saveState();

        // Create a small cube to represent the cursor position

        auto redMaterial = Material::create();
        redMaterial->setDiffuseColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        // Set some OpenGL state variables

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Force filled polygons
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        return 0;
    }

    virtual void onUpdate()
    {}

    virtual void onAddMenuItems(UiMenu *menu) override
    {}

    virtual void onDrawUi() override
    {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    }

    void onExit()
    {
        // Handle exit action, e.g., close the window or application
        this->close();
    }

    void onMousePosition3D(double x, double y, double z)
    {
        std::cout << "3D Mouse Position: (" << x << ", " << y << ", " << z << ")\n";

        // m_cursor->setPos(glm::vec3(x, y, z));
    }
};

typedef std::shared_ptr<EditorWindow> EditorWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = EditorWindow::create(1280, 800, "Editor");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
