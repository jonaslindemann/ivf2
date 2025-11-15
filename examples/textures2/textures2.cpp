/**
 * @file textures2.cpp
 * @brief Multitexturing example
 * @author Jonas Lindemann
 * @example textures2.cpp
 * @ingroup general_examples
 *
 * This example demonstrates the use of multitexturing in a 3D scene using the IVF library.
 * It shows how to apply multiple textures to objects with different blend modes and blend factors.
 * The example includes interactive controls to toggle individual texture layers and adjust blend factors.
 * It features a camera manipulator for navigating the scene and a UI for interaction.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

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
    TextureManagerPtr m_texMgr;
    LightManagerPtr m_lightMgr;

    // Sphere with multitexturing
    SpherePtr m_sphere;
    BoxPtr m_box;
    MaterialPtr m_boxMaterial;
    
    // Individual texture references for control
    TexturePtr m_diffuseTexture;
    TexturePtr m_detailTexture;
    TexturePtr m_lightmapTexture;
    
    // UI state
    bool m_showDemoWindow{false};
    bool m_showControlPanel{true};
    
    // Texture layer controls
    bool m_enableLayer1{true};  // Diffuse
    bool m_enableLayer2{true};  // Detail
    bool m_enableLayer3{true};  // Lightmap
    
    float m_blendFactor1{1.0f};
    float m_blendFactor2{0.5f};
    float m_blendFactor3{0.7f};
    
    int m_blendMode1{0}; // Normal
    int m_blendMode2{1}; // Multiply
    int m_blendMode3{2}; // Add
    
    // Material color control
    float m_materialColor[3]{1.0f, 1.0f, 1.0f}; // RGB
    
    // Global texture blend mode (how final texture result blends with lighting)
    int m_globalBlendMode{1}; // Default to Multiply
    float m_globalBlendFactor{1.0f};

public:
    ExampleWindow(int width, int height, std::string title)
        : GLFWWindow(width, height, title), m_showDemoWindow(false), m_scene(nullptr), m_camManip(nullptr),
          m_texMgr(nullptr), m_lightMgr(nullptr)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Create and load the basic shader using the ShaderManager.

        ShaderManagerPtr shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        // Check for shader compilation and linking errors.

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Create the LightManager and add a point light to the scene.

        m_lightMgr = LightManager::create();

        // Configure the point light's properties.

        auto pointLight1 = m_lightMgr->addPointLight();
        pointLight1->setEnabled(true);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.0, 0.0);
        pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));
        m_lightMgr->apply();

        // Create the TextureManager and enable texture usage.

        m_texMgr = TextureManager::create();
        m_texMgr->setUseTexture(true);
        m_texMgr->setGlobalMultitexturingEnabled(true);
        m_texMgr->setTextureBlendMode(TextureBlendMode::Multiply); // Set to Multiply so textures blend with material
        m_texMgr->setBlendFactor(1.0f);
        m_texMgr->apply();

        // Create the root scene node.

        m_scene = CompositeNode::create();

        // Add axis and grid helpers to the scene.

        auto axis = Axis::create();
        m_scene->add(axis);

        // Create a material for the sphere and enable texture usage.

        m_boxMaterial = Material::create();
        m_boxMaterial->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // White material to show textures clearly
        m_boxMaterial->setAmbientColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
        m_boxMaterial->setSpecularColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
        m_boxMaterial->setUseTexture(true);
        m_boxMaterial->setShininess(100.0);

        // Load textures for multitexturing demonstration
        
        // Layer 1: Base diffuse texture
        m_diffuseTexture = Texture::create();
        m_diffuseTexture->load("assets/test_texture.png");
        m_diffuseTexture->setBlendMode(TextureBlendMode::Normal);
        m_diffuseTexture->setBlendFactor(1.0f);

        // Layer 2: Detail texture (adds fine details)
        m_detailTexture = Texture::create();
        m_detailTexture->load("assets/pop_cat.png");
        m_detailTexture->setBlendMode(TextureBlendMode::Multiply);
        m_detailTexture->setBlendFactor(0.5f);

        // Layer 3: Lightmap texture (adds lighting variation)
        m_lightmapTexture = Texture::create();
        m_lightmapTexture->load("assets/brick.png"); // Reuse for demo
        m_lightmapTexture->setBlendMode(TextureBlendMode::Add);
        m_lightmapTexture->setBlendFactor(0.7f);

        // Create a sphere with multitexturing
        m_box = Box::create();
        m_box->setMaterial(m_boxMaterial);
        m_box->setPos(glm::vec3(0.0, 0.0, 0.0));
        m_box->setScale(glm::vec3(2.0, 2.0, 2.0));
        
        // Add all texture layers
        m_box->addTexture(m_diffuseTexture);
        m_box->addTexture(m_detailTexture);
        m_box->addTexture(m_lightmapTexture);
        
        // Enable multitexturing
        m_box->setUseMultiTexturing(true);

        // Add sphere to scene
        m_scene->add(m_box);

        // Create and configure the camera manipulator.

        m_camManip = CameraManipulator::create(this->ref());
        m_camManip->setCameraPosition(glm::vec3(0.0, 0.0, 10.0));
        
        // Initialize the texture layers to match UI state
        // This ensures textures are properly configured on startup
        updateTextureLayers();

        return 0;
    }

    void updateTextureLayers()
    {
        // Clear and rebuild texture stack based on UI settings
        m_box->clearTextures();
        
        if (m_enableLayer1) {
            m_diffuseTexture->setBlendMode(static_cast<TextureBlendMode>(m_blendMode1));
            m_diffuseTexture->setBlendFactor(m_blendFactor1);
            m_box->addTexture(m_diffuseTexture);
        }
        
        if (m_enableLayer2) {
            m_detailTexture->setBlendMode(static_cast<TextureBlendMode>(m_blendMode2));
            m_detailTexture->setBlendFactor(m_blendFactor2);
            m_box->addTexture(m_detailTexture);
        }
        
        if (m_enableLayer3) {
            m_lightmapTexture->setBlendMode(static_cast<TextureBlendMode>(m_blendMode3));
            m_lightmapTexture->setBlendFactor(m_blendFactor3);
            m_box->addTexture(m_lightmapTexture);
        }
        
        // Update multitexturing state
        // If we have more than 1 texture, enable multitexturing
        // If we have exactly 1 texture, disable multitexturing (use backward compatible path)
        // If we have 0 textures, disable both
        if (m_box->textureCount() > 1) {
            m_box->setUseMultiTexturing(true);
            m_box->setUseTexture(true);
        } else if (m_box->textureCount() == 1) {
            m_box->setUseMultiTexturing(false);
            m_box->setUseTexture(true);
        } else {
            m_box->setUseMultiTexturing(false);
            m_box->setUseTexture(false);
        }
    }

    void onDraw()
    {
        // Clear the screen and depth buffer.

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the scene graph.

        m_scene->draw();
    }

    void onUpdateOtherUi()
    {
        // Update the camera manipulator state.

        m_camManip->update();
        
    }

    void onDrawUi()
    {
        // Draw control panel
        if (m_showControlPanel)
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

            if (ImGui::Begin("Multitexture Control Panel", &m_showControlPanel))
            {
                ImGui::Text("Multitexturing Example");
                ImGui::Separator();

                ImGui::TextWrapped("This example demonstrates layering multiple textures with different blend modes.");
                ImGui::Spacing();

                ImGui::Text("Active Layers: %zu", m_box->textureCount());
                ImGui::Text("Multitexturing: %s", m_box->useMultiTexturing() ? "ON" : "OFF");
                ImGui::Separator();
                
                // Material Color Control
                if (ImGui::CollapsingHeader("Material Settings", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::ColorEdit3("Diffuse Color", m_materialColor))
                    {
                        m_boxMaterial->setDiffuseColor(glm::vec4(
                            m_materialColor[0],
                            m_materialColor[1],
                            m_materialColor[2],
                            1.0f
                        ));
                    }
                    ImGui::TextWrapped("Change the material's diffuse color to see how it affects the final result when combined with textures.");
                    
                    ImGui::Spacing();
                    ImGui::Text("Global Texture Blending:");
                    
                    bool globalChanged = false;
                    const char *globalBlendModes[] = {"Normal", "Multiply", "Add", "Screen", "Overlay", "Decal"};
                    globalChanged |= ImGui::Combo("Global Blend Mode", &m_globalBlendMode, globalBlendModes, 6);
                    globalChanged |= ImGui::SliderFloat("Global Blend Factor", &m_globalBlendFactor, 0.0f, 1.0f);
                    
                    if (globalChanged) {
                        m_texMgr->setTextureBlendMode(static_cast<TextureBlendMode>(m_globalBlendMode));
                        m_texMgr->setBlendFactor(m_globalBlendFactor);
                    }
                    
                    ImGui::TextWrapped("Global blend mode controls how the final multitexture result blends with the lit material color. Use Multiply to tint textures with material color.");
                }

                ImGui::Separator();

                // Layer 1 Controls
                if (ImGui::CollapsingHeader("Layer 1: Base Diffuse", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    bool changed = false;
                    changed |= ImGui::Checkbox("Enable##Layer1", &m_enableLayer1);

                    if (m_enableLayer1)
                    {
                        const char *blendModes[] = {"Normal", "Multiply", "Add", "Screen", "Overlay", "Decal"};
                        changed |= ImGui::Combo("Blend Mode##1", &m_blendMode1, blendModes, 6);
                        changed |= ImGui::SliderFloat("Blend Factor##1", &m_blendFactor1, 0.0f, 1.0f);
                    }

                    if (changed)
                        updateTextureLayers();
                }

                // Layer 2 Controls
                if (ImGui::CollapsingHeader("Layer 2: Detail Texture", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    bool changed = false;
                    changed |= ImGui::Checkbox("Enable##Layer2", &m_enableLayer2);

                    if (m_enableLayer2)
                    {
                        const char *blendModes[] = {"Normal", "Multiply", "Add", "Screen", "Overlay", "Decal"};
                        changed |= ImGui::Combo("Blend Mode##2", &m_blendMode2, blendModes, 6);
                        changed |= ImGui::SliderFloat("Blend Factor##2", &m_blendFactor2, 0.0f, 1.0f);
                    }

                    if (changed)
                        updateTextureLayers();
                }

                // Layer 3 Controls
                if (ImGui::CollapsingHeader("Layer 3: Lightmap", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    bool changed = false;
                    changed |= ImGui::Checkbox("Enable##Layer3", &m_enableLayer3);

                    if (m_enableLayer3)
                    {
                        const char *blendModes[] = {"Normal", "Multiply", "Add", "Screen", "Overlay", "Decal"};
                        changed |= ImGui::Combo("Blend Mode##3", &m_blendMode3, blendModes, 6);
                        changed |= ImGui::SliderFloat("Blend Factor##3", &m_blendFactor3, 0.0f, 1.0f);
                    }

                    if (changed)
                        updateTextureLayers();
                }

                ImGui::Separator();
                ImGui::TextWrapped("The material's diffuse color is multiplied with the lighting result, then textures are blended on top using the specified blend modes.");

                ImGui::Spacing();
                if (ImGui::Button("Reset to Defaults"))
                {
                    m_enableLayer1 = true;
                    m_enableLayer2 = true;
                    m_enableLayer3 = true;
                    m_blendFactor1 = 1.0f;
                    m_blendFactor2 = 0.5f;
                    m_blendFactor3 = 0.7f;
                    m_blendMode1 = 0;
                    m_blendMode2 = 1;
                    m_blendMode3 = 2;
                    m_materialColor[0] = m_materialColor[1] = m_materialColor[2] = 1.0f;
                    m_boxMaterial->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    m_globalBlendMode = 1; // Multiply
                    m_globalBlendFactor = 1.0f;
                    m_texMgr->setTextureBlendMode(TextureBlendMode::Multiply);
                    m_texMgr->setBlendFactor(1.0f);
                    updateTextureLayers();
                }

                ImGui::Separator();
            }
            ImGui::End();
        }

        // Show keyboard shortcuts
        ImGui::SetNextWindowPos(ImVec2(10, 620), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 140), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Keyboard Shortcuts"))
        {
            ImGui::Text("ESC - Close window");
            ImGui::Text("F1  - Toggle control panel");
            ImGui::Text("1-6 - Quick blend mode presets");
            ImGui::Text("Mouse drag - Rotate camera");
            ImGui::Spacing();
            ImGui::TextWrapped("Tip: Try setting material to red/green/blue and see how it combines with textures!");
        }
        ImGui::End();
    }

    void onResize(int width, int height)
    {
        // Update the camera manipulator with the new window size.

        m_camManip->update();
    }

    void onKey(int key, int scancode, int action, int mods)
    {
        // Handle key events for closing the window and toggling UI

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            this->close();
        
        if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
            m_showControlPanel = !m_showControlPanel;

        // Quick preset blend modes for experimentation
        if (action == GLFW_PRESS) {
            bool changed = false;
            
            switch(key) {
                case '1': // Normal for all
                    m_blendMode1 = m_blendMode2 = m_blendMode3 = 0;
                    changed = true;
                    break;
                case '2': // Multiply for all
                    m_blendMode1 = m_blendMode2 = m_blendMode3 = 1;
                    changed = true;
                    break;
                case '3': // Add for all
                    m_blendMode1 = m_blendMode2 = m_blendMode3 = 2;
                    changed = true;
                    break;
                case '4': // Screen for all
                    m_blendMode1 = m_blendMode2 = m_blendMode3 = 3;
                    changed = true;
                    break;
                case '5': // Overlay for all
                    m_blendMode1 = m_blendMode2 = m_blendMode3 = 4;
                    changed = true;
                    break;
                case '6': // Decal for all
                    m_blendMode1 = m_blendMode2 = m_blendMode3 = 5;
                    changed = true;
                    break;
            }
            
            if (changed) {
                updateTextureLayers();
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

    auto window = ExampleWindow::create(1280, 800, "Multitexturing Example");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
