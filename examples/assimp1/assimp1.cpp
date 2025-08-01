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

#include "ImGuiFileDialog.h"
#include "ImGuiFileDialogConfig.h"

using namespace ivf;
using namespace ivfui;
using namespace ivfmath;

using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    bool m_openFileDialog{false};

    SceneInspectorPtr m_sceneInspector{nullptr};

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        // Enable a headlight for basic scene illumination

        this->enableHeadlight();

        // Create and add an axis to the scene for orientation reference

        this->setAxisVisible(true);

        // Setup camera

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0f, 0.0f, 10.0f));
        this->cameraManipulator()->setFarZ(1000.0f);
        this->cameraManipulator()->saveState();

        // Set some OpenGL state variables

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Force filled polygons
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        return 0;
    }

    virtual void onUpdate()
    {}

    virtual void onAddMenuItems(UiMenu *menu) override
    {
        if (menu->name() == "File")
        {
            menu->addItem(UiMenuItem::create("Open", "CTRL+O", [this]() { this->onOpen(); }));
        }
    }

    virtual void onDrawUi() override
    {
        if (m_openFileDialog)
        {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".*", config);
        }

        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                std::string filename = ImGuiFileDialog::Instance()->GetCurrentFileName();

                try
                {
                    auto model = ModelLoader::loadModel(filePathName);

                    if (model)
                    {
                        this->clear(); // Clear previous models if needed
                        this->add(model);
                        std::cout << "Loaded model: " << filename << std::endl;

                        auto wbbox = model->worldBoundingBox();

                        std::cout << "World bounding box: "
                                  << "Min: " << wbbox.min().x << ", " << wbbox.min().y << ", " << wbbox.min().z << "\n"
                                  << "Max: " << wbbox.max().x << ", " << wbbox.max().y << ", " << wbbox.max().z << "\n"
                                  << "Center: " << wbbox.center().x << ", " << wbbox.center().y << ", "
                                  << wbbox.center().z << "\n";

                        if (wbbox.isValid())
                        {
                            // Center the camera on the loaded model
                            this->cameraManipulator()->setCameraTarget(wbbox.center());
                            this->cameraManipulator()->setCameraPosition(
                                glm::vec3(wbbox.center().x, wbbox.center().y, wbbox.max().z * 3.0f));
                            this->cameraManipulator()->setFarZ(wbbox.max().z * 10.0f);
                        }
                    }
                    else
                    {
                        std::cerr << "Failed to load model: " << filename << std::endl;
                    }

                } catch (const std::exception &e)
                {
                    std::cerr << "Error loading file: " << e.what() << std::endl;
                }
            }

            // close
            ImGuiFileDialog::Instance()->Close();
            m_openFileDialog = false;
        }
    }

    void onExit()
    {
        // Handle exit action, e.g., close the window or application
        this->close();
    }

    void onOpen()
    {
        // Handle open action, e.g., load a model file
        m_openFileDialog = true;
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

    auto window = ExampleWindow::create(1280, 800, "Assimp");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
