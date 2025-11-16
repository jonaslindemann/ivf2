/**
 * @file procedural2.cpp
 * @brief Animated GPU Procedural Textures Example
 * @author Jonas Lindemann
 * @example procedural2.cpp
 * @ingroup general_examples
 *
 * This example demonstrates animated GPU-generated procedural textures.
 * It shows how to create and animate checkerboard, noise, and gradient textures
 * in real-time using GPU shaders with time-based animation.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

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
    LightManagerPtr m_lightMgr;

    // Three boxes with different procedural textures
    BoxPtr m_checkerBox;
    BoxPtr m_noiseBox;
    BoxPtr m_gradientBox;
    
    // GPU Procedural Textures
    GPUCheckerboardTexturePtr m_checkerTexture;
    GPUNoiseTexturePtr m_noiseTexture;
    GPUGradientTexturePtr m_gradientTexture;
    
    // Materials
    MaterialPtr m_checkerMaterial;
    MaterialPtr m_noiseMaterial;
    MaterialPtr m_gradientMaterial;
    
    // Animation state
    float m_elapsedTime{0.0f};
    bool m_animateTextures{true};
    float m_animationSpeed{1.0f};
    
    // Texture regeneration control
    bool m_continuousRegeneration{true};
    int m_regenerationInterval{1}; // Regenerate every N frames
    int m_frameCounter{0};
    
    // UI state
    bool m_showControlPanel{true};
    
    // Checkerboard settings
    float m_checkerColor1[4]{1.0f, 1.0f, 1.0f, 1.0f};
    float m_checkerColor2[4]{0.0f, 0.0f, 1.0f, 1.0f};
    float m_checkerSize{8.0f};
    
    // Noise settings
    float m_noiseColorLow[4]{0.2f, 0.1f, 0.0f, 1.0f};
    float m_noiseColorHigh[4]{1.0f, 0.8f, 0.4f, 1.0f};
    float m_noiseScale{4.0f};
    int m_noiseOctaves{4};
    float m_noisePersistence{0.5f};
    float m_noiseLacunarity{2.0f};
    
    // Gradient settings
    float m_gradientColorStart[4]{1.0f, 0.0f, 0.0f, 1.0f};
    float m_gradientColorEnd[4]{0.0f, 1.0f, 1.0f, 1.0f};
    int m_gradientType{0}; // 0=Linear, 1=Radial
    float m_gradientAngle{0.0f};

public:
    ExampleWindow(int width, int height, std::string title)
        : GLFWWindow(width, height, title)
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
        pointLight1->setPosition(glm::vec3(10.0, 10.0, 10.0));
        m_lightMgr->apply();
        
        // Create and configure the TextureManager to enable textures globally
        TextureManagerPtr texMgr = TextureManager::create();
        //texMgr->setUseTexture(true);
        texMgr->setTextureBlendMode(TextureBlendMode::Multiply);
        texMgr->setBlendFactor(1.0f);
        texMgr->apply();

        // Create the root scene node
        m_scene = CompositeNode::create();

        // Add axis and grid helpers
        auto axis = Axis::create();
        m_scene->add(axis);

        // Create GPU Procedural Textures
        createProceduralTextures();

        // Create materials
        createMaterials();

        // Create boxes with procedural textures
        createBoxes();

        // Create and configure the camera manipulator
        m_camManip = CameraManipulator::create(this->ref());
        m_camManip->setCameraPosition(glm::vec3(0.0, 5.0, 15.0));

        return 0;
    }

    void createProceduralTextures()
    {
        // Checkerboard Texture
        m_checkerTexture = GPUCheckerboardTexture::create();
        m_checkerTexture->setSize(512, 512);
        m_checkerTexture->setColors(
            glm::vec4(m_checkerColor1[0], m_checkerColor1[1], m_checkerColor1[2], m_checkerColor1[3]),
            glm::vec4(m_checkerColor2[0], m_checkerColor2[1], m_checkerColor2[2], m_checkerColor2[3])
        );
        m_checkerTexture->setCheckerSize(m_checkerSize);
        m_checkerTexture->regenerate();
        
        // Set texture unit to 0 for proper binding
        m_checkerTexture->setTexUnit(0);

        // Noise Texture
        m_noiseTexture = GPUNoiseTexture::create();
        m_noiseTexture->setSize(512, 512);
        m_noiseTexture->setScale(m_noiseScale);
        m_noiseTexture->setOctaves(m_noiseOctaves);
        m_noiseTexture->setPersistence(m_noisePersistence);
        m_noiseTexture->setLacunarity(m_noiseLacunarity);
        m_noiseTexture->setColorRange(
            glm::vec4(m_noiseColorLow[0], m_noiseColorLow[1], m_noiseColorLow[2], m_noiseColorLow[3]),
            glm::vec4(m_noiseColorHigh[0], m_noiseColorHigh[1], m_noiseColorHigh[2], m_noiseColorHigh[3])
        );
        m_noiseTexture->regenerate();
        
        // Set texture unit to 0 for proper binding
        m_noiseTexture->setTexUnit(0);

        // Gradient Texture
        m_gradientTexture = GPUGradientTexture::create();
        m_gradientTexture->setSize(512, 512);
        m_gradientTexture->setType(GPUGradientTexture::Type::Linear);
        m_gradientTexture->setColors(
            glm::vec4(m_gradientColorStart[0], m_gradientColorStart[1], m_gradientColorStart[2], m_gradientColorStart[3]),
            glm::vec4(m_gradientColorEnd[0], m_gradientColorEnd[1], m_gradientColorEnd[2], m_gradientColorEnd[3])
        );
        m_gradientTexture->setAngle(m_gradientAngle);
        m_gradientTexture->regenerate();
        
        // Set texture unit to 0 for proper binding
        m_gradientTexture->setTexUnit(0);
    }

    void createMaterials()
    {
        m_checkerMaterial = Material::create();
        m_checkerMaterial->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_checkerMaterial->setUseTexture(true);
        m_checkerMaterial->setShininess(50.0);
        m_checkerMaterial->setSpecularColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

        m_noiseMaterial = Material::create();
        m_noiseMaterial->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_noiseMaterial->setUseTexture(true);
        m_noiseMaterial->setShininess(50.0);
        m_noiseMaterial->setSpecularColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

        m_gradientMaterial = Material::create();
        m_gradientMaterial->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_gradientMaterial->setUseTexture(true);
        m_gradientMaterial->setShininess(50.0);
        m_gradientMaterial->setSpecularColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
    }

    void createBoxes()
    {
        // Checkerboard Box (left)
        m_checkerBox = Box::create();
        m_checkerBox->setMaterial(m_checkerMaterial);
        m_checkerBox->setTexture(m_checkerTexture);
        m_checkerBox->setUseTexture(true);  // Explicitly enable textures
        m_checkerBox->setPos(glm::vec3(-5.0, 0.0, 0.0));
        m_checkerBox->setScale(glm::vec3(2.5, 2.5, 2.5));
        m_scene->add(m_checkerBox);

        // Noise Box (center)
        m_noiseBox = Box::create();
        m_noiseBox->setMaterial(m_noiseMaterial);
        m_noiseBox->setTexture(m_noiseTexture);
        m_noiseBox->setUseTexture(true);  // Explicitly enable textures
        m_noiseBox->setPos(glm::vec3(0.0, 0.0, 0.0));
        m_noiseBox->setScale(glm::vec3(2.5, 2.5, 2.5));
        m_scene->add(m_noiseBox);

        // Gradient Box (right)
        m_gradientBox = Box::create();
        m_gradientBox->setMaterial(m_gradientMaterial);
        m_gradientBox->setTexture(m_gradientTexture);
        m_gradientBox->setUseTexture(true);  // Explicitly enable textures
        m_gradientBox->setPos(glm::vec3(5.0, 0.0, 0.0));
        m_gradientBox->setScale(glm::vec3(2.5, 2.5, 2.5));
        m_scene->add(m_gradientBox);
    }

    void updateProceduralTextures(float deltaTime)
    {
        if (m_animateTextures)
        {
            m_elapsedTime += deltaTime * m_animationSpeed;

            // Update time for all textures
            m_checkerTexture->setTime(m_elapsedTime);
            m_noiseTexture->setTime(m_elapsedTime);
            m_gradientTexture->setTime(m_elapsedTime);

            // Regenerate textures based on settings
            if (m_continuousRegeneration)
            {
                // Regenerate every frame
                m_checkerTexture->regenerate();
                m_noiseTexture->regenerate();
                m_gradientTexture->regenerate();
            }
            else
            {
                // Regenerate every N frames
                m_frameCounter++;
                if (m_frameCounter >= m_regenerationInterval)
                {
                    m_frameCounter = 0;
                    m_checkerTexture->regenerate();
                    m_noiseTexture->regenerate();
                    m_gradientTexture->regenerate();
                }
            }
        }
    }

    void updateTextureSettings()
    {
        // Update checkerboard
        m_checkerTexture->setColors(
            glm::vec4(m_checkerColor1[0], m_checkerColor1[1], m_checkerColor1[2], m_checkerColor1[3]),
            glm::vec4(m_checkerColor2[0], m_checkerColor2[1], m_checkerColor2[2], m_checkerColor2[3])
        );
        m_checkerTexture->setCheckerSize(m_checkerSize);

        // Update noise
        m_noiseTexture->setColorRange(
            glm::vec4(m_noiseColorLow[0], m_noiseColorLow[1], m_noiseColorLow[2], m_noiseColorLow[3]),
            glm::vec4(m_noiseColorHigh[0], m_noiseColorHigh[1], m_noiseColorHigh[2], m_noiseColorHigh[3])
        );
        m_noiseTexture->setScale(m_noiseScale);
        m_noiseTexture->setOctaves(m_noiseOctaves);
        m_noiseTexture->setPersistence(m_noisePersistence);
        m_noiseTexture->setLacunarity(m_noiseLacunarity);

        // Update gradient
        m_gradientTexture->setColors(
            glm::vec4(m_gradientColorStart[0], m_gradientColorStart[1], m_gradientColorStart[2], m_gradientColorStart[3]),
            glm::vec4(m_gradientColorEnd[0], m_gradientColorEnd[1], m_gradientColorEnd[2], m_gradientColorEnd[3])
        );
        m_gradientTexture->setType(m_gradientType == 0 ? GPUGradientTexture::Type::Linear : GPUGradientTexture::Type::Radial);
        m_gradientTexture->setAngle(m_gradientAngle);

        // Force regeneration
        m_checkerTexture->regenerate();
        m_noiseTexture->regenerate();
        m_gradientTexture->regenerate();
    }

    void onDraw()
    {
        // Clear the screen and depth buffer
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update procedural textures with animation
        updateProceduralTextures(static_cast<float>(this->frameTime()));

        // Draw the scene graph
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

            if (ImGui::Begin("Animated GPU Procedural Textures", &m_showControlPanel))
            {
                ImGui::Text("GPU Procedural Texture Animation");
                ImGui::Separator();

                // Animation Controls
                if (ImGui::CollapsingHeader("Animation Settings", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox("Animate Textures", &m_animateTextures);
                    ImGui::SliderFloat("Animation Speed", &m_animationSpeed, 0.0f, 5.0f);
                    ImGui::Text("Elapsed Time: %.2f s", m_elapsedTime);
                    
                    ImGui::Spacing();
                    ImGui::Checkbox("Continuous Regeneration", &m_continuousRegeneration);
                    
                    if (!m_continuousRegeneration)
                    {
                        ImGui::SliderInt("Regeneration Interval (frames)", &m_regenerationInterval, 1, 60);
                    }
                    
                    ImGui::Spacing();
                    if (ImGui::Button("Reset Time"))
                    {
                        m_elapsedTime = 0.0f;
                    }
                    
                    ImGui::TextWrapped("Continuous regeneration updates every frame. "
                                      "Interval mode updates every N frames for better performance.");
                }

                ImGui::Separator();

                // Checkerboard Settings
                if (ImGui::CollapsingHeader("Checkerboard Texture", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    bool changed = false;
                    changed |= ImGui::ColorEdit4("Color 1##Checker", m_checkerColor1);
                    changed |= ImGui::ColorEdit4("Color 2##Checker", m_checkerColor2);
                    changed |= ImGui::SliderFloat("Checker Size", &m_checkerSize, 1.0f, 32.0f);
                    
                    if (changed)
                    {
                        updateTextureSettings();
                    }
                    
                    ImGui::TextWrapped("Animated: Scrolling horizontally");
                }

                ImGui::Separator();

                // Noise Settings
                if (ImGui::CollapsingHeader("Noise Texture", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    bool changed = false;
                    changed |= ImGui::ColorEdit4("Low Color##Noise", m_noiseColorLow);
                    changed |= ImGui::ColorEdit4("High Color##Noise", m_noiseColorHigh);
                    changed |= ImGui::SliderFloat("Scale", &m_noiseScale, 0.5f, 16.0f);
                    changed |= ImGui::SliderInt("Octaves", &m_noiseOctaves, 1, 8);
                    changed |= ImGui::SliderFloat("Persistence", &m_noisePersistence, 0.0f, 1.0f);
                    changed |= ImGui::SliderFloat("Lacunarity", &m_noiseLacunarity, 1.0f, 4.0f);
                    
                    if (changed)
                    {
                        updateTextureSettings();
                    }
                    
                    ImGui::TextWrapped("Animated: Flowing diagonally");
                }

                ImGui::Separator();

                // Gradient Settings
                if (ImGui::CollapsingHeader("Gradient Texture", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    bool changed = false;
                    changed |= ImGui::ColorEdit4("Start Color##Gradient", m_gradientColorStart);
                    changed |= ImGui::ColorEdit4("End Color##Gradient", m_gradientColorEnd);
                    
                    const char* types[] = {"Linear", "Radial"};
                    changed |= ImGui::Combo("Type", &m_gradientType, types, 2);
                    
                    if (m_gradientType == 0)
                    {
                        changed |= ImGui::SliderFloat("Angle (rad)", &m_gradientAngle, 0.0f, 6.28318f);
                    }
                    
                    if (changed)
                    {
                        updateTextureSettings();
                    }
                    
                    if (m_gradientType == 0)
                        ImGui::TextWrapped("Animated: Rotating");
                    else
                        ImGui::TextWrapped("Animated: Pulsing center");
                }

                ImGui::Separator();
                
                if (ImGui::Button("Reset All Settings"))
                {
                    // Reset to defaults
                    m_checkerColor1[0] = m_checkerColor1[1] = m_checkerColor1[2] = m_checkerColor1[3] = 1.0f;
                    m_checkerColor2[0] = m_checkerColor2[1] = m_checkerColor2[2] = 0.0f;
                    m_checkerColor2[3] = 1.0f;
                    m_checkerSize = 8.0f;
                    
                    m_noiseColorLow[0] = 0.2f; m_noiseColorLow[1] = 0.1f;
                    m_noiseColorLow[2] = 0.0f; m_noiseColorLow[3] = 1.0f;
                    m_noiseColorHigh[0] = 1.0f; m_noiseColorHigh[1] = 0.8f;
                    m_noiseColorHigh[2] = 0.4f; m_noiseColorHigh[3] = 1.0f;
                    m_noiseScale = 4.0f;
                    m_noiseOctaves = 4;
                    m_noisePersistence = 0.5f;
                    m_noiseLacunarity = 2.0f;
                    
                    m_gradientColorStart[0] = 1.0f; m_gradientColorStart[1] = 0.0f;
                    m_gradientColorStart[2] = 0.0f; m_gradientColorStart[3] = 1.0f;
                    m_gradientColorEnd[0] = 0.0f; m_gradientColorEnd[1] = 1.0f;
                    m_gradientColorEnd[2] = 1.0f; m_gradientColorEnd[3] = 1.0f;
                    m_gradientType = 0;
                    m_gradientAngle = 0.0f;
                    
                    m_animationSpeed = 1.0f;
                    m_elapsedTime = 0.0f;
                    
                    updateTextureSettings();
                }
            }
            ImGui::End();
        }

        // Keyboard shortcuts window
        ImGui::SetNextWindowPos(ImVec2(10, 720), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(450, 100), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Controls"))
        {
            ImGui::Text("ESC - Close window");
            ImGui::Text("F1  - Toggle control panel");
            ImGui::Text("SPACE - Play/Pause animation");
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

        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
            m_animateTextures = !m_animateTextures;
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

    auto window = ExampleWindow::create(1280, 800, "Animated GPU Procedural Textures");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
