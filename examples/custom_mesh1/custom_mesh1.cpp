/**
 * @file custom_mesh1.cpp
 * @brief DynamicMesh for generative geometry — Lissajous curve and Lorenz attractor.
 * @ingroup mesh_examples
 *
 * Shows how to build and stream-update a DynamicMesh each frame:
 *  - Lissajous figure-8 as a GL_LINE_STRIP
 *  - Lorenz attractor as a GL_POINTS cloud, updated in real time
 *
 * ImGui lets you switch between the two and tweak parameters.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;

class MeshWindow : public GLFWSceneWindow {
private:
    DynamicMeshPtr m_mesh;
    int    m_mode{0};      // 0=Lissajous, 1=Lorenz
    float  m_t{0.0f};
    int    m_segments{512};
    float  m_aFreq{3.0f}, m_bFreq{2.0f}, m_delta{float(M_PI)/4.0f};

    // Lorenz state
    std::vector<glm::vec3> m_lorenz;
    float m_lx{1}, m_ly{1}, m_lz{1};
    static constexpr float k_sigma{10}, k_rho{28}, k_beta{8.0f/3.0f};

public:
    MeshWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<MeshWindow> create(int w, int h, std::string title)
    { return std::make_shared<MeshWindow>(w, h, title); }

    int onSetup() override
    {
        m_mesh = DynamicMesh::create();
        m_mesh->setLineWidth(2.0f);

        auto mat = Material::create();
        mat->setDiffuseColor({0.2f, 0.9f, 1.0f, 1.0f});
        mat->setAmbientColor({0.05f,0.3f,0.4f,1.0f});
        m_mesh->setMaterial(mat);
        add(m_mesh);

        buildLissajous();

        enableAxis();
        cameraManipulator()->setCameraPosition({0, 5, 30});
        return 0;
    }

    void buildLissajous()
    {
        std::vector<glm::vec3> pts(m_segments + 1);
        for (int i = 0; i <= m_segments; ++i) {
            float u = float(i) / float(m_segments) * 2.0f * float(M_PI);
            pts[i] = { std::sin(m_aFreq * u + m_delta) * 5.0f,
                       std::sin(m_bFreq * u) * 5.0f,
                       0.0f };
        }

        m_mesh->beginMesh(GL_LINE_STRIP);
        for (auto& p : pts) m_mesh->vertex(p);
        m_mesh->endMesh();
    }

    void buildLorenz()
    {
        m_mesh->beginMesh(GL_POINTS);
        for (auto& p : m_lorenz) m_mesh->vertex(p);
        m_mesh->endMesh();
        m_mesh->setPointSize(2.0f);
    }

    void onUpdate() override
    {
        float dt = float(frameTime());
        m_t += dt;

        if (m_mode == 1) {
            float h = 0.005f;
            for (int i = 0; i < 5; ++i) {
                float dx = k_sigma * (m_ly - m_lx);
                float dy = m_lx * (k_rho - m_lz) - m_ly;
                float dz = m_lx * m_ly - k_beta * m_lz;
                m_lx += dx * h;
                m_ly += dy * h;
                m_lz += dz * h;
                m_lorenz.push_back({m_lx * 0.3f, m_lz * 0.3f - 8.0f, m_ly * 0.3f});
                if (m_lorenz.size() > 8000) m_lorenz.erase(m_lorenz.begin());
            }
            buildLorenz();
        } else {
            // Animate Lissajous delta
            m_delta = m_t * 0.3f;
            buildLissajous();
        }
    }

    void onDrawUi() override
    {
        ImGui::Begin("DynamicMesh");
        int prevMode = m_mode;
        ImGui::RadioButton("Lissajous", &m_mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Lorenz Attractor", &m_mode, 1);

        if (m_mode == 0) {
            ImGui::SliderFloat("a freq", &m_aFreq, 1, 7);
            ImGui::SliderFloat("b freq", &m_bFreq, 1, 7);
            ImGui::SliderInt("Segments", &m_segments, 64, 2048);
        }

        if (prevMode != m_mode && m_mode == 1) {
            m_lorenz.clear();
            m_lx = m_ly = m_lz = 1;
        }

        ImGui::Text("Vertices: %d", m_mesh->vertexCount());
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = MeshWindow::create(1024, 768, "DynamicMesh — Generative Geometry");
    win->maximize();
    app->addWindow(win);
    return app->loop();
}
