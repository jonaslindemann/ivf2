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
        this->setRenderToTexture(true);
        this->enableHeadlight();

        auto axis = Axis::create();

        m_material = Material::create();
        m_material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_material->setDiffuseColor(glm::vec4(0.8, 0.8, 0.0, 1.0));
        m_material->setAmbientColor(glm::vec4(0.2, 0.2, 0.0, 1.0));

        GridLayout layout(30, 1, 30, 1.2, 0.0, 1.2);

        auto box = RoundedBox::create(glm::vec3(0.52, 0.52, 0.52), glm::vec3(8, 8, 8), 0.16);

        m_nodes = CompositeNode::create();

        for (auto i = 0; i < layout.size(); i++)
        {
            auto material = Material::create();
            material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
            material->setDiffuseColor(glm::vec4(random(0.2, 1.0), random(0.2, 1.0), random(0.2, 1.0), 1.0));
            material->setAmbientColor(glm::vec4(0.2, 0.2, 0.0, 1.0));

            auto xfm = Transform::create();
            xfm->setUseMaterial(true);
            xfm->add(box);

            xfm->setMaterial(material);

            m_nodes->add(xfm);
        }

        m_nodes->setPos(glm::vec3(0.0, -4.0, 0.0));

        layout.apply(m_nodes);

        m_nodes->storeChildrenPos();

        this->add(m_nodes);
        // this->add(axis);

        this->scene()->accept(&m_extentVisitor);

        auto bbox = m_extentVisitor.bbox();

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0, bbox.size().y / 2.0, 2 * bbox.min().z));
        this->cameraManipulator()->setCameraTarget(bbox.center());

        auto blurEffect = BlurEffect::create();
        blurEffect->setBlurRadius(2.0);
        blurEffect->load();

        auto tintEffect = TintEffect::create();
        tintEffect->setTintColor(glm::vec3(1.2, 0.9, 0.7));
        tintEffect->setTintStrength(0.5);
        tintEffect->setGrayScaleWeights(glm::vec3(0.299, 0.587, 0.114));
        tintEffect->load();

        auto filmgrainEffect = FilmgrainEffect::create();
        filmgrainEffect->setNoiseIntensity(0.5);
        filmgrainEffect->setGrainBlending(0.5);
        filmgrainEffect->load();

        auto chromaticEffect = ChromaticEffect::create();
        chromaticEffect->setOffset(0.01);
        chromaticEffect->load();

        auto vignetteEffect = VignetteEffect::create();
        vignetteEffect->setSize(1.0);
        vignetteEffect->setSmoothness(0.7);
        vignetteEffect->load();

        auto bloomEffect = BloomEffect::create();
        bloomEffect->setThreshold(1.0);
        bloomEffect->setIntensity(1.0);
        bloomEffect->load();

        auto ditheringEffect = DitheringEffect::create();
        ditheringEffect->load();

        auto pixelationEffect = PixelationEffect::create();
        pixelationEffect->setPixelSize(4.0);
        pixelationEffect->load();

        this->addEffect(blurEffect); // OK
        this->addEffect(tintEffect);
        this->addEffect(chromaticEffect); // OK
        this->addEffect(ditheringEffect); // OK
        this->addEffect(bloomEffect);     // OK
        this->addEffect(pixelationEffect);
        this->addEffect(vignetteEffect);
        this->addEffect(filmgrainEffect); // OK

        this->disableAllEffects();

        m_inspector = ObjectInspector::create("Inspector");
        this->addUiWindow(m_inspector);

        return 0;
    }

    virtual void onUpdate()
    {
        m_nodes->accept(&m_visitor);
    }

    virtual void onKey(int key, int scancode, int action, int mods) override
    {
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

    auto window = ExampleWindow::create(800, 800, "Example 6");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
