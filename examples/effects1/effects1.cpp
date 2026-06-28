/**
 * @file effects1.cpp
 * @brief Post-processing effects example
 * @author Jonas Lindemann
 * @example effects1.cpp
 * @ingroup effect_examples
 *
 * This example demonstrates the use of various post-processing effects
 * including blur, tint, film grain, chromatic aberration, vignette,
 * bloom, dithering, pixelation, wave distortion, swirl, kaleidoscope,
 * glitch, scanline, posterize, color grading, night vision, feedback/trails,
 * motion blur, and halftone.
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
#include <ivf/wave_distortion_effect.h>
#include <ivf/swirl_effect.h>
#include <ivf/kaleidoscope_effect.h>
#include <ivf/glitch_effect.h>
#include <ivf/scanline_effect.h>
#include <ivf/posterize_effect.h>
#include <ivf/color_grading_effect.h>
#include <ivf/night_vision_effect.h>
#include <ivf/feedback_effect.h>
#include <ivf/motion_blur_effect.h>
#include <ivf/halftone_effect.h>

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
        // Enable a headlight for the scene

        this->enableHeadlight();

        // Enable rendering to texture for post-processing effects

        this->setRenderToTexture(true);

        // Create an axis (not added to the scene by default)

        // Create a base material for the objects

        m_material = Material::create();
        m_material->setSpecularColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        m_material->setDiffuseColor(glm::vec4(0.8, 0.8, 0.0, 1.0));
        m_material->setAmbientColor(glm::vec4(0.2, 0.2, 0.0, 1.0));

        // Set up a grid layout for the objects (30x30 grid, spacing 1.2)

        GridLayout layout(30, 30, 1, 1.2, 0.0, 1.2);

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

        // Wave distortion effect

        auto waveDistortionEffect = WaveDistortionEffect::create();
        waveDistortionEffect->setFrequency(10.0f);
        waveDistortionEffect->setAmplitude(0.01f);
        waveDistortionEffect->setSpeed(1.0f);
        waveDistortionEffect->load();

        // Swirl effect

        auto swirlEffect = SwirlEffect::create();
        swirlEffect->setRadius(0.5f);
        swirlEffect->setAngle(3.14159f);
        swirlEffect->load();

        // Kaleidoscope effect

        auto kaleidoscopeEffect = KaleidoscopeEffect::create();
        kaleidoscopeEffect->setSegments(6.0f);
        kaleidoscopeEffect->setRotation(0.0f);
        kaleidoscopeEffect->load();

        // Glitch effect

        auto glitchEffect = GlitchEffect::create();
        glitchEffect->setIntensity(0.05f);
        glitchEffect->setBlockSize(0.05f);
        glitchEffect->setSpeed(4.0f);
        glitchEffect->load();

        // Scanline effect

        auto scanlineEffect = ScanlineEffect::create();
        scanlineEffect->setLineSpacing(4.0f);
        scanlineEffect->setLineIntensity(0.4f);
        scanlineEffect->setScrollSpeed(0.0f);
        scanlineEffect->load();

        // Posterize effect

        auto posterizeEffect = PosterizeEffect::create();
        posterizeEffect->setLevels(4.0f);
        posterizeEffect->load();

        // Color grading effect

        auto colorGradingEffect = ColorGradingEffect::create();
        colorGradingEffect->setShadows(glm::vec3(0.6f, 0.5f, 0.5f));
        colorGradingEffect->setMidtones(glm::vec3(0.5f, 0.5f, 0.5f));
        colorGradingEffect->setHighlights(glm::vec3(0.5f, 0.5f, 0.6f));
        colorGradingEffect->setContrast(1.1f);
        colorGradingEffect->setSaturation(1.2f);
        colorGradingEffect->load();

        // Night vision effect

        auto nightVisionEffect = NightVisionEffect::create();
        nightVisionEffect->setNoiseIntensity(0.05f);
        nightVisionEffect->setGlowStrength(0.3f);
        nightVisionEffect->setPhosphorColor(glm::vec3(0.1f, 1.0f, 0.1f));
        nightVisionEffect->load();

        // Feedback / trails effect

        auto feedbackEffect = FeedbackEffect::create();
        feedbackEffect->setFeedbackAmount(0.9f);
        feedbackEffect->setZoom(1.02f);
        feedbackEffect->setRotation(0.01f);
        feedbackEffect->setColorDecay(glm::vec3(0.98f));
        feedbackEffect->load();

        // Motion blur effect

        auto motionBlurEffect = MotionBlurEffect::create();
        motionBlurEffect->setBlendFactor(0.7f);
        motionBlurEffect->load();

        // Halftone effect

        auto halftoneEffect = HalftoneEffect::create();
        halftoneEffect->setDotSize(6.0f);
        halftoneEffect->setAngle(0.0f);
        halftoneEffect->setGrayscale(false);
        halftoneEffect->load();

        // Add effects to the window (order matters for compositing)

        this->addEffect(blurEffect);            //  0
        this->addEffect(tintEffect);            //  1
        this->addEffect(chromaticEffect);       //  2
        this->addEffect(ditheringEffect);       //  3
        this->addEffect(bloomEffect);           //  4
        this->addEffect(pixelationEffect);      //  5
        this->addEffect(vignetteEffect);        //  6
        this->addEffect(filmgrainEffect);       //  7
        this->addEffect(waveDistortionEffect);  //  8
        this->addEffect(swirlEffect);           //  9
        this->addEffect(kaleidoscopeEffect);    // 10
        this->addEffect(glitchEffect);          // 11
        this->addEffect(scanlineEffect);        // 12
        this->addEffect(posterizeEffect);       // 13
        this->addEffect(colorGradingEffect);    // 14
        this->addEffect(nightVisionEffect);     // 15
        this->addEffect(feedbackEffect);        // 16
        this->addEffect(motionBlurEffect);      // 17
        this->addEffect(halftoneEffect);        // 18

        // Disable all effects by default

        this->disableAllEffects();

        // Create and add the object inspector UI window

        this->showEffectInspector();

        return 0;
    }

    virtual void onUpdate()
    {
        // Update the function visitor to animate the nodes

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

    auto window = ExampleWindow::create(1280, 800, "Example 6");
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
