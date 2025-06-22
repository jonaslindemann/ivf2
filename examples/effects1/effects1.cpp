/**
 * @file effects1.cpp
 * @brief Deformer example
 * @author Jonas Lindemann
 * @example effects1.cpp
 * @ingroup effect_examples
 *
 * This example demonstrates the use of various post-processing effects
 * including blur, tint, film grain, chromatic aberration, vignette,
 * bloom, dithering, and pixelation.
 */

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <ivf/node_visitor.h>
#include <ivf/stock_shaders.h>
#include <ivf/blur_effect.h>
#include <ivf/spline_animation.h>
#include <ivf/keyframe_animation.h>
#include <ivf/model_loader.h>

#include <ivf/tint_effect.h>
#include <ivf/filmgrain_effect.h>
#include <ivf/chromatic_effect.h>
#include <ivf/vignette_effect.h>
#include <ivf/bloom_effect.h>
#include <ivf/dithering_effect.h>
#include <ivf/pixelation_effect.h>

#include <ivfmath/spline.h>

#include <ivf/grid_layout.h>
#include <ivf/extent_visitor.h>

#include <ivfui/object_inspector.h>

using namespace ivf;
using namespace ivfui;
using namespace ivfmath;

using namespace std;

// Visitor to apply transformations and effects to nodes

class FunctionVisitor : public NodeVisitor {
private:
public:
    virtual void visit(Node *node) override
    {
        auto xfm = dynamic_cast<Transform *>(node);
        if (xfm == nullptr)
            return;

        auto storedPos = xfm->storedPos();
        auto pos = xfm->pos();

        auto center = glm::vec3(0.0, 0.0, 0.0);

        auto l = glm::length(pos - center);

        pos.y = storedPos.y + 0.5 * sin(0.5 * l + glfwGetTime());

        xfm->setPos(pos);

        auto material = xfm->material();

        if (material)
        {
            auto color = material->diffuseColor();
            color.r = 0.5 + 0.5 * sin(l + glfwGetTime());
            color.g = 0.5 + 0.5 * sin(l + glfwGetTime() + 2.0);
            color.b = 0.5 + 0.5 * sin(l + glfwGetTime() + 4.0);
            material->setDiffuseColor(color);
        }
    }
};

// Main example window class

class ExampleWindow : public GLFWSceneWindow {
private:
    FunctionVisitor m_visitor;
    ExtentVisitor m_extentVisitor;
    CompositeNodePtr m_nodes;
    MaterialPtr m_material;

    ObjectInspectorPtr m_inspector;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        // Enable rendering to texture for post-processing effects

        this->setRenderToTexture(true);

        // Enable a headlight for the scene

        this->enableHeadlight();

        // Create an axis (not added to the scene by default)

        auto axis = Axis::create();

        // Create a base material for the objects

        m_material = Material::create();
        m_material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_material->setDiffuseColor(glm::vec4(0.8, 0.8, 0.0, 1.0));
        m_material->setAmbientColor(glm::vec4(0.2, 0.2, 0.0, 1.0));

        // Set up a grid layout for the objects (30x30 grid, spacing 1.2)

        GridLayout layout(30, 1, 30, 1.2, 0.0, 1.2);

        // Create a rounded box geometry

        auto box = RoundedBox::create(glm::vec3(0.52, 0.52, 0.52), glm::vec3(8, 8, 8), 0.16);

        // Create a composite node to hold all objects

        m_nodes = CompositeNode::create();

        // Populate the grid with transform nodes, each with a random colored material

        for (auto i = 0; i < layout.size(); i++)
        {
            // Create a new material with random diffuse color

            auto material = Material::create();
            material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
            material->setDiffuseColor(glm::vec4(random(0.2, 1.0), random(0.2, 1.0), random(0.2, 1.0), 1.0));
            material->setAmbientColor(glm::vec4(0.2, 0.2, 0.0, 1.0));

            // Create a transform node and assign the material

            auto xfm = Transform::create();
            xfm->setUseMaterial(true);
            xfm->add(box);

            xfm->setMaterial(material);

            // Add the transform node to the composite node

            m_nodes->add(xfm);
        }

        // Position the grid of nodes

        m_nodes->setPos(glm::vec3(0.0, -4.0, 0.0));

        // Apply the grid layout to the composite node

        layout.apply(m_nodes);

        // Store the initial positions of the children for animation

        m_nodes->storeChildrenPos();

        // Add the composite node to the scene

        this->add(m_nodes);
        // this->add(axis); // Uncomment to add axis to the scene

        // Compute the bounding box of the scene

        this->scene()->accept(&m_extentVisitor);

        auto bbox = m_extentVisitor.bbox();

        // Set up the camera to view the scene

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0, bbox.size().y / 2.0, 2 * bbox.min().z));
        this->cameraManipulator()->setCameraTarget(bbox.center());

        // Create and configure post-processing effects

        // Blur effect

        auto blurEffect = BlurEffect::create();
        blurEffect->setBlurRadius(2.0);
        blurEffect->load();

        // Tint effect

        auto tintEffect = TintEffect::create();
        tintEffect->setTintColor(glm::vec3(1.2, 0.9, 0.7));
        tintEffect->setTintStrength(0.5);
        tintEffect->setGrayScaleWeights(glm::vec3(0.299, 0.587, 0.114));
        tintEffect->load();

        // Film grain effect

        auto filmgrainEffect = FilmgrainEffect::create();
        filmgrainEffect->setNoiseIntensity(0.5);
        filmgrainEffect->setGrainBlending(0.5);
        filmgrainEffect->load();

        // Chromatic aberration effect

        auto chromaticEffect = ChromaticEffect::create();
        chromaticEffect->setOffset(0.01);
        chromaticEffect->load();

        // Vignette effect

        auto vignetteEffect = VignetteEffect::create();
        vignetteEffect->setSize(1.0);
        vignetteEffect->setSmoothness(0.7);
        vignetteEffect->load();

        // Bloom effect

        auto bloomEffect = BloomEffect::create();
        bloomEffect->setThreshold(1.0);
        bloomEffect->setIntensity(1.0);
        bloomEffect->load();

        // Dithering effect

        auto ditheringEffect = DitheringEffect::create();
        ditheringEffect->load();

        // Pixelation effect

        auto pixelationEffect = PixelationEffect::create();
        pixelationEffect->setPixelSize(4.0);
        pixelationEffect->load();

        // Add effects to the window (order matters for compositing)

        this->addEffect(blurEffect);       // 0
        this->addEffect(tintEffect);       // 1
        this->addEffect(chromaticEffect);  // 2
        this->addEffect(ditheringEffect);  // 3
        this->addEffect(bloomEffect);      // 4
        this->addEffect(pixelationEffect); // 5
        this->addEffect(vignetteEffect);   // 6
        this->addEffect(filmgrainEffect);  // 7

        // Disable all effects by default

        this->disableAllEffects();

        // Create and add the object inspector UI window

        m_inspector = ObjectInspector::create("Inspector");
        this->addUiWindow(m_inspector);

        return 0;
    }

    virtual void onUpdate()
    {
        // Update the function visitor to animate the nodes

        m_nodes->accept(&m_visitor);
    }

    virtual void onKey(int key, int scancode, int action, int mods) override
    {
        // Handle key events for toggling effects and showing UI

        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_0)
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    m_inspector->setObject(this->effect(0));
                }
                else
                {
                    if (this->isEffectEnabled(0))
                        this->disableEffect(0);
                    else
                        this->enableEffect(0);
                }
            }
            else if (key == GLFW_KEY_1)
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    m_inspector->setObject(this->effect(1));
                }
                else
                {
                    if (this->isEffectEnabled(1))
                        this->disableEffect(1);
                    else
                        this->enableEffect(1);
                }
            }
            else if (key == GLFW_KEY_2)
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    m_inspector->setObject(this->effect(2));
                }
                else
                {
                    if (this->isEffectEnabled(2))
                        this->disableEffect(2);
                    else
                        this->enableEffect(2);
                }
            }
            else if (key == GLFW_KEY_3)
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    m_inspector->setObject(this->effect(3));
                }
                else
                {
                    if (this->isEffectEnabled(3))
                        this->disableEffect(3);
                    else
                        this->enableEffect(3);
                }
            }
            else if (key == GLFW_KEY_4)
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    m_inspector->setObject(this->effect(4));
                }
                else
                {
                    if (this->isEffectEnabled(4))
                        this->disableEffect(4);
                    else
                        this->enableEffect(4);
                }
            }
            else if (key == GLFW_KEY_5)
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    m_inspector->setObject(this->effect(5));
                }
                else
                {
                    if (this->isEffectEnabled(5))
                        this->disableEffect(5);
                    else
                        this->enableEffect(5);
                }
            }
            else if (key == GLFW_KEY_6)
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    m_inspector->setObject(this->effect(6));
                }
                else
                {
                    if (this->isEffectEnabled(6))
                        this->disableEffect(6);
                    else
                        this->enableEffect(6);
                }
            }
            else if (key == GLFW_KEY_7)
            {
                if (mods & GLFW_MOD_SHIFT)
                {
                    m_inspector->setObject(this->effect(7));
                }
                else
                {
                    if (this->isEffectEnabled(7))
                        this->disableEffect(7);
                    else
                        this->enableEffect(7);
                }
            }
            if (key == GLFW_KEY_ESCAPE)
            {
                this->close();
            }
            else if (key == GLFW_KEY_F1)
            {
                this->showControlPanel();
            }
            else if (key == GLFW_KEY_F2)
            {
                this->showCameraWindow();
            }
        }
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

    auto window = ExampleWindow::create(1280, 800, "Example 6");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
