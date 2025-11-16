/**
 * @file textures3.cpp
 * @brief GPU Procedural texture generation example
 * @author Jonas Lindemann
 * @example textures3.cpp
 * @ingroup general_examples
 *
 * This example demonstrates GPU-based procedural texture generation in IVF2.
 * It showcases real-time procedural texture generation using fragment shaders
 * and render-to-texture techniques. Users can interactively switch between
 * different texture types and adjust their parameters with immediate visual feedback.
 */

#include <iostream>
#include <memory>
#include <string>

#include <imgui.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/gpu_procedural_texture.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWWindow {
private:
    CompositeNodePtr m_scene;
    CameraManipulatorPtr m_camManip;
    TextureManagerPtr m_texMgr;
    LightManagerPtr m_lightMgr;

    // Geometry
    BoxPtr m_box;
    MaterialPtr m_material;
    
    // GPU Procedural textures
    GPUCheckerboardTexturePtr m_checkerTexture;
    GPUNoiseTexturePtr m_noiseTexture;
    GPUGradientTexturePtr m_gradientTexture;
    
    // Current texture pointer
    GPUProceduralTexturePtr m_currentTexture;
    
    // UI state
    bool m_showControlPanel{true};
    int m_currentTextureType{0}; // 0=Checker, 1=Noise, 2=Gradient
    
    // Shared parameters
    int m_textureSize[2]{512, 512};
    
    // Checkerboard parameters
    float m_checkerSize{8.0f};
    float m_checkerColor1[4]{1.0f, 1.0f, 1.0f, 1.0f};
    float m_checkerColor2[4]{0.0f, 0.0f, 0.0f, 1.0f};
    
    // Noise parameters
    float m_noiseScale{4.0f};
    int m_noiseOctaves{4};
    float m_noisePersistence{0.5f};
    float m_noiseLacunarity{2.0f};
    float m_noiseColorLow[4]{0.0f, 0.0f, 0.0f, 1.0f};
    float m_noiseColorHigh[4]{1.0f, 1.0f, 1.0f, 1.0f};
    
    // Gradient parameters
    int m_gradientType{0}; // 0=Linear, 1=Radial
    float m_gradientAngle{0.0f};
    float m_gradientCenter[2]{0.5f, 0.5f};
    float m_gradientColorStart[4]{0.0f, 0.0f, 0.0f, 1.0f};
    float m_gradientColorEnd[4]{1.0f, 1.0f, 1.0f, 1.0f};

public:
    ExampleWindow(int width, int height, std::string title)
        : GLFWWindow(width, height, title), m_scene(nullptr), m_camManip(nullptr),
          m_texMgr(nullptr), m_lightMgr(nullptr)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Create and load the basic shader
        ShaderManagerPtr shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Create the LightManager and add a point light
        m_lightMgr = LightManager::create();

        auto pointLight1 = m_lightMgr->addPointLight();
        pointLight1->setEnabled(true);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.0, 0.0);
        pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));
        m_lightMgr->apply();

        // Create the TextureManager and enable texture usage
        m_texMgr = TextureManager::create();
        m_texMgr->setUseTexture(true);
        m_texMgr->apply();

        // Create the root scene node
        m_scene = CompositeNode::create();

        // Add axis helper
        auto axis = Axis::create();
        m_scene->add(axis);

        // Create material
        m_material = Material::create();
        m_material->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_material->setAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
        m_material->setSpecularColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
        m_material->setUseTexture(true);
        m_material->setShininess(100.0);

        // Create GPU procedural textures
        m_checkerTexture = GPUCheckerboardTexture::create();
        m_checkerTexture->setSize(512, 512);
        m_checkerTexture->setCheckerSize(8.0f);
        m_checkerTexture->regenerate();

        m_noiseTexture = GPUNoiseTexture::create();
        m_noiseTexture->setSize(512, 512);
        m_noiseTexture->setScale(4.0f);
        m_noiseTexture->setOctaves(4);
        m_noiseTexture->regenerate();

        m_gradientTexture = GPUGradientTexture::create();
        m_gradientTexture->setSize(512, 512);
        m_gradientTexture->setType(GPUGradientTexture::Type::Linear);
        m_gradientTexture->regenerate();

        // Create box with checkerboard texture as default
        m_box = Box::create();
        m_box->setMaterial(m_material);
        m_box->setPos(glm::vec3(0.0, 0.0, 0.0));
        m_box->setScale(glm::vec3(2.0, 2.0, 2.0));
        m_box->setUseTexture(true);
        m_box->addTexture(m_checkerTexture);
        m_scene->add(m_box);

        m_currentTexture = m_checkerTexture;

        // Create camera manipulator
        m_camManip = CameraManipulator::create(this->ref());
        m_camManip->setCameraPosition(glm::vec3(0.0, 0.0, 8.0));

        return 0;
    }

    void updateTexture()
    {
        // Clear current textures
        m_box->clearTextures();

        // Switch to selected texture type
        switch (m_currentTextureType)
        {
        case 0: // Checkerboard
            m_currentTexture = m_checkerTexture;
            break;
        case 1: // Noise
            m_currentTexture = m_noiseTexture;
            break;
        case 2: // Gradient
            m_currentTexture = m_gradientTexture;
            break;
        }

        // Add the selected texture
        m_box->addTexture(m_currentTexture);
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

    void onDrawUi()
    {
        if (m_showControlPanel)
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(450, 650), ImGuiCond_FirstUseEver);

            if (ImGui::Begin("GPU Procedural Texture Control Panel", &m_showControlPanel))
            {
                ImGui::Text("GPU Procedural Texture Generator");
                ImGui::Separator();

                ImGui::TextWrapped("This example demonstrates GPU-based procedural texture generation using fragment shaders and render-to-texture.");
                ImGui::Spacing();

                ImGui::Text("Current texture: %dx%d", m_currentTexture->width(), m_currentTexture->height());
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "GPU-Generated (Real-time!)");
                ImGui::Separator();

                // Texture type selector
                const char *textureTypes[] = {"Checkerboard", "Perlin Noise", "Gradient"};
                if (ImGui::Combo("Texture Type", &m_currentTextureType, textureTypes, 3))
                {
                    updateTexture();
                }

                ImGui::Separator();

                // Global texture size
                if (ImGui::InputInt2("Texture Size", m_textureSize))
                {
                    m_textureSize[0] = std::max(64, std::min(4096, m_textureSize[0]));
                    m_textureSize[1] = std::max(64, std::min(4096, m_textureSize[1]));
                    m_currentTexture->setSize(m_textureSize[0], m_textureSize[1]);
                    m_currentTexture->regenerate();
                }

                ImGui::Spacing();

                // Type-specific parameters
                if (m_currentTextureType == 0) // Checkerboard
                {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Checkerboard Parameters");
                    
                    if (ImGui::SliderFloat("Checker Size", &m_checkerSize, 2.0f, 32.0f))
                    {
                        m_checkerTexture->setCheckerSize(m_checkerSize);
                        m_checkerTexture->regenerate();
                    }
                    
                    if (ImGui::ColorEdit4("Color 1", m_checkerColor1))
                    {
                        m_checkerTexture->setColors(
                            glm::vec4(m_checkerColor1[0], m_checkerColor1[1], m_checkerColor1[2], m_checkerColor1[3]),
                            glm::vec4(m_checkerColor2[0], m_checkerColor2[1], m_checkerColor2[2], m_checkerColor2[3])
                        );
                        m_checkerTexture->regenerate();
                    }
                    
                    if (ImGui::ColorEdit4("Color 2", m_checkerColor2))
                    {
                        m_checkerTexture->setColors(
                            glm::vec4(m_checkerColor1[0], m_checkerColor1[1], m_checkerColor1[2], m_checkerColor1[3]),
                            glm::vec4(m_checkerColor2[0], m_checkerColor2[1], m_checkerColor2[2], m_checkerColor2[3])
                        );
                        m_checkerTexture->regenerate();
                    }
                }
                else if (m_currentTextureType == 1) // Noise
                {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Perlin Noise Parameters");
                    
                    if (ImGui::SliderFloat("Scale", &m_noiseScale, 0.1f, 20.0f))
                    {
                        m_noiseTexture->setScale(m_noiseScale);
                        m_noiseTexture->regenerate();
                    }
                    
                    if (ImGui::SliderInt("Octaves", &m_noiseOctaves, 1, 8))
                    {
                        m_noiseTexture->setOctaves(m_noiseOctaves);
                        m_noiseTexture->regenerate();
                    }
                    
                    if (ImGui::SliderFloat("Persistence", &m_noisePersistence, 0.1f, 0.9f))
                    {
                        m_noiseTexture->setPersistence(m_noisePersistence);
                        m_noiseTexture->regenerate();
                    }
                    
                    if (ImGui::SliderFloat("Lacunarity", &m_noiseLacunarity, 1.0f, 4.0f))
                    {
                        m_noiseTexture->setLacunarity(m_noiseLacunarity);
                        m_noiseTexture->regenerate();
                    }
                    
                    if (ImGui::ColorEdit4("Color Low", m_noiseColorLow))
                    {
                        m_noiseTexture->setColorRange(
                            glm::vec4(m_noiseColorLow[0], m_noiseColorLow[1], m_noiseColorLow[2], m_noiseColorLow[3]),
                            glm::vec4(m_noiseColorHigh[0], m_noiseColorHigh[1], m_noiseColorHigh[2], m_noiseColorHigh[3])
                        );
                        m_noiseTexture->regenerate();
                    }
                    
                    if (ImGui::ColorEdit4("Color High", m_noiseColorHigh))
                    {
                        m_noiseTexture->setColorRange(
                            glm::vec4(m_noiseColorLow[0], m_noiseColorLow[1], m_noiseColorLow[2], m_noiseColorLow[3]),
                            glm::vec4(m_noiseColorHigh[0], m_noiseColorHigh[1], m_noiseColorHigh[2], m_noiseColorHigh[3])
                        );
                        m_noiseTexture->regenerate();
                    }
                }
                else if (m_currentTextureType == 2) // Gradient
                {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Gradient Parameters");
                    
                    const char *gradientTypes[] = {"Linear", "Radial"};
                    if (ImGui::Combo("Gradient Type", &m_gradientType, gradientTypes, 2))
                    {
                        m_gradientTexture->setType(m_gradientType == 0 ? 
                            GPUGradientTexture::Type::Linear : 
                            GPUGradientTexture::Type::Radial);
                        m_gradientTexture->regenerate();
                    }
                    
                    if (m_gradientType == 0) // Linear
                    {
                        if (ImGui::SliderAngle("Angle", &m_gradientAngle))
                        {
                            m_gradientTexture->setAngle(m_gradientAngle);
                            m_gradientTexture->regenerate();
                        }
                    }
                    else // Radial
                    {
                        if (ImGui::SliderFloat2("Center", m_gradientCenter, 0.0f, 1.0f))
                        {
                            m_gradientTexture->setCenter(glm::vec2(m_gradientCenter[0], m_gradientCenter[1]));
                            m_gradientTexture->regenerate();
                        }
                    }
                    
                    if (ImGui::ColorEdit4("Start Color", m_gradientColorStart))
                    {
                        m_gradientTexture->setColors(
                            glm::vec4(m_gradientColorStart[0], m_gradientColorStart[1], m_gradientColorStart[2], m_gradientColorStart[3]),
                            glm::vec4(m_gradientColorEnd[0], m_gradientColorEnd[1], m_gradientColorEnd[2], m_gradientColorEnd[3])
                        );
                        m_gradientTexture->regenerate();
                    }
                    
                    if (ImGui::ColorEdit4("End Color", m_gradientColorEnd))
                    {
                        m_gradientTexture->setColors(
                            glm::vec4(m_gradientColorStart[0], m_gradientColorStart[1], m_gradientColorStart[2], m_gradientColorStart[3]),
                            glm::vec4(m_gradientColorEnd[0], m_gradientColorEnd[1], m_gradientColorEnd[2], m_gradientColorEnd[3])
                        );
                        m_gradientTexture->regenerate();
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Performance Note:");
                ImGui::TextWrapped("GPU textures regenerate in ~1ms even at 2048x2048! Try changing the size to see real-time performance.");
                
                ImGui::Spacing();
                if (ImGui::Button("Reset to Defaults"))
                {
                    m_checkerSize = 8.0f;
                    m_checkerColor1[0] = m_checkerColor1[1] = m_checkerColor1[2] = m_checkerColor1[3] = 1.0f;
                    m_checkerColor2[0] = m_checkerColor2[1] = m_checkerColor2[2] = 0.0f;
                    m_checkerColor2[3] = 1.0f;
                    
                    m_noiseScale = 4.0f;
                    m_noiseOctaves = 4;
                    m_noisePersistence = 0.5f;
                    m_noiseLacunarity = 2.0f;
                    
                    m_gradientType = 0;
                    m_gradientAngle = 0.0f;
                    
                    m_currentTexture->regenerate();
                }
            }
            ImGui::End();
        }

        // Keyboard shortcuts window
        ImGui::SetNextWindowPos(ImVec2(10, 670), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(450, 110), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Keyboard Shortcuts"))
        {
            ImGui::Text("ESC - Close window");
            ImGui::Text("F1  - Toggle control panel");
            ImGui::Text("Mouse drag - Rotate camera");
            ImGui::Spacing();
            ImGui::TextWrapped("Tip: GPU textures are MUCH faster than CPU! Compare with procedural1 example.");
        }
        ImGui::End();
    }

    void onResize(int width, int height)
    {
        m_camManip->update();
    }

    void onKey(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            this->close();
        
        if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
            m_showControlPanel = !m_showControlPanel;
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

    auto window = ExampleWindow::create(1280, 800, "GPU Procedural Textures Example");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
