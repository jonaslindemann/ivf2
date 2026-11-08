/**
 * @file instancing1.cpp
 * @brief GPU instancing with InstancedMeshNode.
 * @ingroup general_examples
 *
 * Demonstrates InstancedMeshNode:
 *  - A grid of up to 40 000 lit cubes drawn with one draw call, using the
 *    stock shader with per-instance colors
 *  - All instance transforms updated every frame to animate a wave
 *  - A ring of glowing spheres using a ShaderMaterial that reads the
 *    per-instance data (a phase) in the vertex shader
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <cmath>

using namespace ivf;
using namespace ivfui;

class InstancingWindow : public GLFWSceneWindow {
private:
    InstancedMeshNodePtr m_cubes;
    InstancedMeshNodePtr m_orbs;
    int m_gridSize{100};
    int m_builtGridSize{0};
    bool m_animate{true};

    void buildGrid()
    {
        m_cubes->setInstanceCount(size_t(m_gridSize) * m_gridSize);
        size_t i = 0;
        for (int x = 0; x < m_gridSize; ++x)
            for (int z = 0; z < m_gridSize; ++z, ++i)
            {
                float u = float(x) / m_gridSize, v = float(z) / m_gridSize;
                m_cubes->setInstanceColor(i, {0.3f + 0.7f * u, 0.4f, 0.3f + 0.7f * v, 1.0f});
            }
        m_builtGridSize = m_gridSize;
        updateGrid(0.0f);
    }

    void updateGrid(float t)
    {
        const float spacing = 0.3f;
        const float offset = -0.5f * spacing * (m_gridSize - 1);
        size_t i = 0;
        for (int x = 0; x < m_gridSize; ++x)
            for (int z = 0; z < m_gridSize; ++z, ++i)
            {
                float px = offset + x * spacing, pz = offset + z * spacing;
                float r = std::sqrt(px * px + pz * pz);
                float h = 0.6f + 0.5f * std::sin(r * 0.8f - t * 2.0f);
                m_cubes->setInstanceTransform(
                    i, InstancedMeshNode::makeTransform({px, h * 0.5f, pz}, {0.0f, 0.0f, 0.0f}, {0.2f, h, 0.2f}));
            }
    }

public:
    InstancingWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<InstancingWindow> create(int w, int h, std::string title)
    { return std::make_shared<InstancingWindow>(w, h, title); }

    int onSetup() override
    {
        auto light = LightManager::instance()->addDirectionalLight();
        light->setDiffuseColor({1.0f, 0.95f, 0.9f});
        light->setDirection({-0.4f, -1.0f, -0.3f});
        light->setEnabled(true);

        auto cubeMaterial = Material::create();
        cubeMaterial->setDiffuseColor({1.0f, 1.0f, 1.0f, 1.0f});

        m_cubes = InstancedMeshNode::create(Cube::create());
        m_cubes->setMaterial(cubeMaterial);
        add(m_cubes);
        buildGrid();

        auto glow = ShaderMaterial::createFromFiles("shaders/instanced_glow.vert", "shaders/instanced_glow.frag",
                                                    "instanced_glow");
        if (!glow)
            return -1;

        m_orbs = InstancedMeshNode::create(Sphere::create(0.4, 24, 12));
        m_orbs->setMaterial(glow);
        const int orbCount = 64;
        for (int i = 0; i < orbCount; ++i)
        {
            float a = 6.2831853f * i / orbCount;
            glm::vec3 pos{18.0f * std::cos(a), 3.0f + std::sin(a * 3.0f), 18.0f * std::sin(a)};
            glm::vec4 color{0.5f + 0.5f * std::cos(a), 0.6f, 0.5f + 0.5f * std::sin(a), 1.0f};
            m_orbs->addInstance(InstancedMeshNode::makeTransform(pos), color, glm::vec4(a * 4.0f, 0, 0, 0));
        }
        add(m_orbs);

        setClearColor({0.02f, 0.02f, 0.05f, 1.0f});
        cameraManipulator()->setCameraPosition({0.0f, 18.0f, 32.0f});
        return 0;
    }

    void onUpdate() override
    {
        ShaderMaterial::setGlobalUniform("time", float(elapsedTime()));

        if (m_gridSize != m_builtGridSize)
            buildGrid();
        if (m_animate)
            updateGrid(float(elapsedTime()));
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({300, 130}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Instancing");
        ImGui::SliderInt("Grid size", &m_gridSize, 10, 200);
        ImGui::Text("Cubes: %zu", m_cubes->instanceCount());
        ImGui::Checkbox("Animate", &m_animate);
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto win = InstancingWindow::create(1280, 720, "Instancing");
    app->addWindow(win);
    return app->loop();
}
