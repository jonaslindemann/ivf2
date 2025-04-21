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

        // this->addEffect(blurEffect); // OK
        // this->addEffect(tintEffect);
        // this->addEffect(chromaticEffect); // OK
        // this->addEffect(ditheringEffect); // OK
        // this->addEffect(bloomEffect); // OK
        // this->addEffect(pixelationEffect);
        // this->addEffect(vignetteEffect);
        this->addEffect(filmgrainEffect); // OK

        return 0;
    }

    virtual void onUpdate()
    {
        m_nodes->accept(&m_visitor);
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
