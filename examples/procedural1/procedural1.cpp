/**
 * @file procedural1.cpp
 * @brief Procedural texture generation example
 * @author Jonas Lindemann
 * @example procedural1.cpp
 * @ingroup general_examples
 *
 * This example demonstrates the use of procedurally generated textures in IVF2.
 * It showcases various procedural texture types including checkerboard, Perlin noise,
 * gradients, marble, and wood patterns. Users can interactively switch between different
 * texture types and adjust their parameters in real-time.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/procedural_texture.h>
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
    
    // Procedural textures
    CheckerboardTexturePtr m_checkerTexture;
    PerlinNoiseTexturePtr m_noiseTexture;
    GradientTexturePtr m_gradientTexture;
    MarbleTexturePtr m_marbleTexture;
    WoodTexturePtr m_woodTexture;
    
    // Current texture pointer
    ProceduralTexturePtr m_currentTexture;
    
    // UI state
    bool m_showControlPanel{true};
    int m_currentTextureType{0}; // 0=Checker, 1=Noise, 2=Gradient, 3=Marble, 4=Wood
    
    // Shared parameters
    int m_textureSize[2]{512, 512};
    
    // Checkerboard parameters
    int m_checkerSize{32};
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
    
    // Marble parameters
    float m_marbleScale{1.0f};
    int m_marbleOctaves{4};
    float m_marbleColor1[4]{0.9f, 0.9f, 0.9f, 1.0f};
    float m_marbleColor2[4]{0.3f, 0.3f, 0.3f, 1.0f};
    
    // Wood parameters
    float m_woodScale{1.0f};
    float m_woodRings{10.0f};
    float m_woodColor1[4]{0.6f, 0.4f, 0.2f, 1.0f};
    float m_woodColor2[4]{0.3f, 0.2f, 0.1f, 1.0f};

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

        // Create procedural textures
        m_checkerTexture = CheckerboardTexture::create();
        m_checkerTexture->setSize(512, 512);
        m_checkerTexture->setCheckerSize(32);
        m_checkerTexture->regenerate();

        m_noiseTexture = PerlinNoiseTexture::create();
        m_noiseTexture->setSize(512, 512);
        m_noiseTexture->setScale(4.0f);
        m_noiseTexture->setOctaves(4);
        m_noiseTexture->regenerate();

        m_gradientTexture = GradientTexture::create();
        m_gradientTexture->setSize(512, 512);
        m_gradientTexture->setType(GradientTexture::GradientType::Linear);
        m_gradientTexture->regenerate();

        m_marbleTexture = MarbleTexture::create();
        m_marbleTexture->setSize(512, 512);
        m_marbleTexture->setScale(1.0f);
        m_marbleTexture->regenerate();

        m_woodTexture = WoodTexture::create();
        m_woodTexture->setSize(512, 512);
        m_woodTexture->setScale(1.0f);
        m_woodTexture->setRings(10.0f);
        m_woodTexture->regenerate();

        // Create box with checkerboard texture as default
        m_box = Box::create();
        m_box->setMaterial(m_material);
        m_box->setPos(glm::vec3(0.0, 0.0, 0.0));
        m_box->setScale(glm::vec3(2.0, 2.0, 2.0));
        m_box->setUseTexture(true); // Enable texture usage on the box
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
        case 3: // Marble
            m_currentTexture = m_marbleTexture;
            break;
        case 4: // Wood
            m_currentTexture = m_woodTexture;
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
            ImGui::SetNextWindowSize(ImVec2(450, 700), ImGuiCond_FirstUseEver);

            if (ImGui::Begin("Procedural Texture Control Panel", &m_showControlPanel))
            {
                ImGui::Text("Procedural Texture Generator");
                ImGui::Separator();

                ImGui::TextWrapped("This example demonstrates CPU-based procedural texture generation with various patterns and parameters.");
                ImGui::Spacing();

                // Texture type selector
                const char *textureTypes[] = {"Checkerboard", "Perlin Noise", "Gradient", "Marble", "Wood"};
                if (ImGui::Combo("Texture Type", &m_currentTextureType, textureTypes, 5))
                {
                    updateTexture();
                }

                ImGui::Separator();

                // Global texture size
                if (ImGui::InputInt2("Texture Size", m_textureSize))
                {
                    m_textureSize[0] = std::max(64, std::min(2048, m_textureSize[0]));
                    m_textureSize[1] = std::max(64, std::min(2048, m_textureSize[1]));
                    m_currentTexture->setSize(m_textureSize[0], m_textureSize[1]);
                    m_currentTexture->regenerate(); // Regenerate when size changes
                }

                ImGui::Spacing();

                // Type-specific parameters
                bool needsRegeneration = false;

                if (m_currentTextureType == 0) // Checkerboard
                {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Checkerboard Parameters");
                    
                    if (ImGui::SliderInt("Checker Size", &m_checkerSize, 4, 128))
                    {
                        m_checkerTexture->setCheckerSize(m_checkerSize);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("Color 1", m_checkerColor1))
                    {
                        m_checkerTexture->setColors(
                            glm::vec4(m_checkerColor1[0], m_checkerColor1[1], m_checkerColor1[2], m_checkerColor1[3]),
                            glm::vec4(m_checkerColor2[0], m_checkerColor2[1], m_checkerColor2[2], m_checkerColor2[3])
                        );
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("Color 2", m_checkerColor2))
                    {
                        m_checkerTexture->setColors(
                            glm::vec4(m_checkerColor1[0], m_checkerColor1[1], m_checkerColor1[2], m_checkerColor1[3]),
                            glm::vec4(m_checkerColor2[0], m_checkerColor2[1], m_checkerColor2[2], m_checkerColor2[3])
                        );
                        needsRegeneration = true;
                    }
                }
                else if (m_currentTextureType == 1) // Noise
                {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Perlin Noise Parameters");
                    
                    if (ImGui::SliderFloat("Scale", &m_noiseScale, 0.1f, 20.0f))
                    {
                        m_noiseTexture->setScale(m_noiseScale);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::SliderInt("Octaves", &m_noiseOctaves, 1, 8))
                    {
                        m_noiseTexture->setOctaves(m_noiseOctaves);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::SliderFloat("Persistence", &m_noisePersistence, 0.1f, 0.9f))
                    {
                        m_noiseTexture->setPersistence(m_noisePersistence);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::SliderFloat("Lacunarity", &m_noiseLacunarity, 1.0f, 4.0f))
                    {
                        m_noiseTexture->setLacunarity(m_noiseLacunarity);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("Color Low", m_noiseColorLow))
                    {
                        m_noiseTexture->setColorRange(
                            glm::vec4(m_noiseColorLow[0], m_noiseColorLow[1], m_noiseColorLow[2], m_noiseColorLow[3]),
                            glm::vec4(m_noiseColorHigh[0], m_noiseColorHigh[1], m_noiseColorHigh[2], m_noiseColorHigh[3])
                        );
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("Color High", m_noiseColorHigh))
                    {
                        m_noiseTexture->setColorRange(
                            glm::vec4(m_noiseColorLow[0], m_noiseColorLow[1], m_noiseColorLow[2], m_noiseColorLow[3]),
                            glm::vec4(m_noiseColorHigh[0], m_noiseColorHigh[1], m_noiseColorHigh[2], m_noiseColorHigh[3])
                        );
                        needsRegeneration = true;
                    }
                }
                else if (m_currentTextureType == 2) // Gradient
                {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Gradient Parameters");
                    
                    const char *gradientTypes[] = {"Linear", "Radial"};
                    if (ImGui::Combo("Gradient Type", &m_gradientType, gradientTypes, 2))
                    {
                        m_gradientTexture->setType(m_gradientType == 0 ? 
                            GradientTexture::GradientType::Linear : 
                            GradientTexture::GradientType::Radial);
                        needsRegeneration = true;
                    }
                    
                    if (m_gradientType == 0) // Linear
                    {
                        if (ImGui::SliderAngle("Angle", &m_gradientAngle))
                        {
                            m_gradientTexture->setAngle(m_gradientAngle);
                            needsRegeneration = true;
                        }
                    }
                    else // Radial
                    {
                        if (ImGui::SliderFloat2("Center", m_gradientCenter, 0.0f, 1.0f))
                        {
                            m_gradientTexture->setCenter(glm::vec2(m_gradientCenter[0], m_gradientCenter[1]));
                            needsRegeneration = true;
                        }
                    }
                    
                    if (ImGui::ColorEdit4("Start Color", m_gradientColorStart))
                    {
                        m_gradientTexture->setColors(
                            glm::vec4(m_gradientColorStart[0], m_gradientColorStart[1], m_gradientColorStart[2], m_gradientColorStart[3]),
                            glm::vec4(m_gradientColorEnd[0], m_gradientColorEnd[1], m_gradientColorEnd[2], m_gradientColorEnd[3])
                        );
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("End Color", m_gradientColorEnd))
                    {
                        m_gradientTexture->setColors(
                            glm::vec4(m_gradientColorStart[0], m_gradientColorStart[1], m_gradientColorStart[2], m_gradientColorStart[3]),
                            glm::vec4(m_gradientColorEnd[0], m_gradientColorEnd[1], m_gradientColorEnd[2], m_gradientColorEnd[3])
                        );
                        needsRegeneration = true;
                    }
                }
                else if (m_currentTextureType == 3) // Marble
                {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Marble Parameters");
                    
                    if (ImGui::SliderFloat("Scale", &m_marbleScale, 0.1f, 10.0f))
                    {
                        m_marbleTexture->setScale(m_marbleScale);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::SliderInt("Octaves", &m_marbleOctaves, 1, 8))
                    {
                        m_marbleTexture->setOctaves(m_marbleOctaves);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("Light Color", m_marbleColor1))
                    {
                        m_marbleTexture->setColors(
                            glm::vec4(m_marbleColor1[0], m_marbleColor1[1], m_marbleColor1[2], m_marbleColor1[3]),
                            glm::vec4(m_marbleColor2[0], m_marbleColor2[1], m_marbleColor2[2], m_marbleColor2[3])
                        );
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("Dark Color", m_marbleColor2))
                    {
                        m_marbleTexture->setColors(
                            glm::vec4(m_marbleColor1[0], m_marbleColor1[1], m_marbleColor1[2], m_marbleColor1[3]),
                            glm::vec4(m_marbleColor2[0], m_marbleColor2[1], m_marbleColor2[2], m_marbleColor2[3])
                        );
                        needsRegeneration = true;
                    }
                }
                else if (m_currentTextureType == 4) // Wood
                {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Wood Parameters");
                    
                    if (ImGui::SliderFloat("Scale", &m_woodScale, 0.1f, 10.0f))
                    {
                        m_woodTexture->setScale(m_woodScale);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::SliderFloat("Rings", &m_woodRings, 1.0f, 50.0f))
                    {
                        m_woodTexture->setRings(m_woodRings);
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("Light Color", m_woodColor1))
                    {
                        m_woodTexture->setColors(
                            glm::vec4(m_woodColor1[0], m_woodColor1[1], m_woodColor1[2], m_woodColor1[3]),
                            glm::vec4(m_woodColor2[0], m_woodColor2[1], m_woodColor2[2], m_woodColor2[3])
                        );
                        needsRegeneration = true;
                    }
                    
                    if (ImGui::ColorEdit4("Dark Color", m_woodColor2))
                    {
                        m_woodTexture->setColors(
                            glm::vec4(m_woodColor1[0], m_woodColor1[1], m_woodColor1[2], m_woodColor1[3]),
                            glm::vec4(m_woodColor2[0], m_woodColor2[1], m_woodColor2[2], m_woodColor2[3])
                        );
                        needsRegeneration = true;
                    }
                }

                if (needsRegeneration)
                {
                    m_currentTexture->regenerate();
                }

                ImGui::Spacing();
                ImGui::Separator();
                
                if (ImGui::Button("Regenerate Texture"))
                {
                    m_currentTexture->regenerate();
                }

                ImGui::Spacing();
                ImGui::Text("Current texture: %dx%d", m_currentTexture->width(), m_currentTexture->height());
            }
            ImGui::End();
        }

        // Keyboard shortcuts window
        ImGui::SetNextWindowPos(ImVec2(10, 720), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(450, 100), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Keyboard Shortcuts"))
        {
            ImGui::Text("ESC - Close window");
            ImGui::Text("F1  - Toggle control panel");
            ImGui::Text("Mouse drag - Rotate camera");
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

    auto window = ExampleWindow::create(1280, 800, "Procedural Textures Example");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
