/**
 * @file textures4.cpp
 * @brief PBR material showcase — roughness × metallic sphere grid.
 *
 * Demonstrates PBRMaterial on a 5×5 grid of spheres where:
 *   - X axis varies roughness  (0 = mirror-smooth → 1 = fully diffuse)
 *   - Y axis varies metallic   (0 = dielectric   → 1 = full metal)
 *
 * Controls:
 *   - Mouse: orbit camera
 *   - ImGui sliders: albedo colour, ambient occlusion, emissive colour
 *   - "Preset" buttons: gold, iron, plastic, mirror
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;

static constexpr int COLS = 5;
static constexpr int ROWS = 5;
static constexpr float SPACING = 2.2f;

class Textures4Window : public GLFWSceneWindow {
    std::vector<std::vector<PBRMaterialPtr>> m_mats;

    glm::vec4 m_albedo{1.0f, 0.71f, 0.29f, 1.0f}; // gold default
    float     m_ao{1.0f};
    glm::vec3 m_emissive{0.0f};

public:
    Textures4Window(int w, int h, const std::string& title)
        : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<Textures4Window> create(int w, int h, const std::string& t)
    { return std::make_shared<Textures4Window>(w, h, t); }

    int onSetup() override
    {
        cameraManipulator()->setCameraPosition({0, 0, 18});

        m_mats.resize(ROWS, std::vector<PBRMaterialPtr>(COLS));

        float originX = -(COLS - 1) * SPACING * 0.5f;
        float originY = -(ROWS - 1) * SPACING * 0.5f;

        for (int row = 0; row < ROWS; ++row) {
            float metallic = float(row) / float(ROWS - 1);
            for (int col = 0; col < COLS; ++col) {
                float roughness = float(col) / float(COLS - 1);
                roughness = glm::max(roughness, 0.04f);

                auto mat = PBRMaterial::create();
                mat->setAlbedo(m_albedo);
                mat->setRoughness(roughness);
                mat->setMetallic(metallic);
                mat->setAO(m_ao);
                m_mats[row][col] = mat;

                auto sphere = Sphere::create(0.85f, 32, 32);
                sphere->setPos({originX + col * SPACING, originY + row * SPACING, 0.0f});
                sphere->setMaterial(mat);
                add(sphere);
            }
        }

        auto lightMgr = LightManager::instance();
        lightMgr->clearLights();

        // Directional key light
        auto dirLight = lightMgr->addDirectionalLight();
        dirLight->setDirection({-1.0f, -1.0f, -1.0f});
        dirLight->setDiffuseColor({3.0f, 3.0f, 3.0f});
        dirLight->setAmbientColor({0.05f, 0.05f, 0.05f});
        dirLight->setSpecularColor({3.0f, 3.0f, 3.0f});
        dirLight->setEnabled(true);

        // Fill light
        auto fillLight = lightMgr->addPointLight();
        fillLight->setPosition({5.0f, 5.0f, 8.0f});
        fillLight->setDiffuseColor({2.0f, 2.0f, 2.5f});
        fillLight->setAttenuation(1.0f, 0.05f, 0.005f);
        fillLight->setEnabled(true);

        lightMgr->setUseLighting(true);

        return 0;
    }

    void rebuildMaterials()
    {
        for (int row = 0; row < ROWS; ++row) {
            float metallic  = float(row) / float(ROWS - 1);
            for (int col = 0; col < COLS; ++col) {
                float roughness = glm::max(float(col) / float(COLS - 1), 0.04f);
                auto& mat = m_mats[row][col];
                mat->setAlbedo(m_albedo);
                mat->setRoughness(roughness);
                mat->setMetallic(metallic);
                mat->setAO(m_ao);
                mat->setEmissive(m_emissive);
            }
        }
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({10, 10}, ImGuiCond_Once);
        ImGui::SetNextWindowSize({320, 340}, ImGuiCond_Once);
        ImGui::Begin("PBR Material");

        bool dirty = false;

        float col[4] = { m_albedo.r, m_albedo.g, m_albedo.b, m_albedo.a };
        if (ImGui::ColorEdit3("Albedo", col)) {
            m_albedo = { col[0], col[1], col[2], 1.0f };
            dirty = true;
        }

        float em[3] = { m_emissive.r, m_emissive.g, m_emissive.b };
        if (ImGui::ColorEdit3("Emissive", em)) {
            m_emissive = { em[0], em[1], em[2] };
            dirty = true;
        }

        if (ImGui::SliderFloat("AO", &m_ao, 0.0f, 1.0f)) dirty = true;

        ImGui::Separator();
        ImGui::Text("Axis labels:");
        ImGui::Text("  X → Roughness  0 (left) to 1 (right)");
        ImGui::Text("  Y → Metallic   0 (bottom) to 1 (top)");

        ImGui::Separator();
        ImGui::Text("Presets:");
        if (ImGui::Button("Gold"))    { m_albedo = {1.0f, 0.71f, 0.29f, 1.0f}; dirty = true; }
        ImGui::SameLine();
        if (ImGui::Button("Iron"))    { m_albedo = {0.56f, 0.57f, 0.58f, 1.0f}; dirty = true; }
        ImGui::SameLine();
        if (ImGui::Button("Plastic")) { m_albedo = {0.8f, 0.1f, 0.1f, 1.0f}; dirty = true; }
        ImGui::SameLine();
        if (ImGui::Button("Mirror"))  { m_albedo = {0.9f, 0.9f, 0.95f, 1.0f}; dirty = true; }

        ImGui::End();

        if (dirty) rebuildMaterials();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_SAMPLES, 4);

    auto win = Textures4Window::create(1280, 800, "PBR Material Grid — textures4");
    app->addWindow(win);
    return app->loop();
}
