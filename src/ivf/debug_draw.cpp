#include <ivf/debug_draw.h>
#include <ivf/shader_manager.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <cmath>

namespace ivf {

DebugDraw* DebugDraw::m_instance = nullptr;

// ---- Inline shaders --------------------------------------------------------

static const char* k_vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
out vec4 vColor;
uniform mat4 viewProj;
void main() {
    vColor = aColor;
    gl_Position = viewProj * vec4(aPos, 1.0);
}
)";

static const char* k_fragSrc = R"(
#version 330 core
in vec4 vColor;
out vec4 fragColor;
void main() { fragColor = vColor; }
)";

// ---- Init ------------------------------------------------------------------

void DebugDraw::init()
{
    if (m_initialized) return;

    // Compile inline shader
    auto vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &k_vertSrc, nullptr);
    glCompileShader(vert);

    auto frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &k_fragSrc, nullptr);
    glCompileShader(frag);

    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);

    glDeleteShader(vert);
    glDeleteShader(frag);

    // VAO / VBO (dynamic, updated every flush)
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, pos)));
    // color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, color)));

    glBindVertexArray(0);

    m_initialized = true;
}

// ---- Helpers ---------------------------------------------------------------

void DebugDraw::addLine(glm::vec3 p0, glm::vec3 p1, glm::vec4 color)
{
    m_vertices.push_back({p0, color});
    m_vertices.push_back({p1, color});
}

// ---- Public API ------------------------------------------------------------

void DebugDraw::line(glm::vec3 p0, glm::vec3 p1, glm::vec4 color, float duration)
{
    if (duration > 0.0f)
        m_persistent.push_back({p0, p1, color, duration});
    else
        addLine(p0, p1, color);
}

void DebugDraw::box(glm::vec3 center, glm::vec3 hs, glm::vec4 color, float duration)
{
    glm::vec3 corners[8] = {
        center + glm::vec3(-hs.x, -hs.y, -hs.z),
        center + glm::vec3( hs.x, -hs.y, -hs.z),
        center + glm::vec3( hs.x,  hs.y, -hs.z),
        center + glm::vec3(-hs.x,  hs.y, -hs.z),
        center + glm::vec3(-hs.x, -hs.y,  hs.z),
        center + glm::vec3( hs.x, -hs.y,  hs.z),
        center + glm::vec3( hs.x,  hs.y,  hs.z),
        center + glm::vec3(-hs.x,  hs.y,  hs.z),
    };
    auto edge = [&](int a, int b){ line(corners[a], corners[b], color, duration); };
    // Bottom ring
    edge(0,1); edge(1,2); edge(2,3); edge(3,0);
    // Top ring
    edge(4,5); edge(5,6); edge(6,7); edge(7,4);
    // Verticals
    edge(0,4); edge(1,5); edge(2,6); edge(3,7);
}

void DebugDraw::sphere(glm::vec3 center, float radius, glm::vec4 color, float duration, int segments)
{
    const float pi2 = glm::pi<float>() * 2.0f;
    // Three great circles: XY, XZ, YZ
    for (int ring = 0; ring < 3; ++ring) {
        glm::vec3 prev{};
        for (int i = 0; i <= segments; ++i) {
            float a = pi2 * i / segments;
            glm::vec3 p;
            if (ring == 0)      p = center + glm::vec3(std::cos(a), std::sin(a), 0) * radius;
            else if (ring == 1) p = center + glm::vec3(std::cos(a), 0, std::sin(a)) * radius;
            else                p = center + glm::vec3(0, std::cos(a), std::sin(a)) * radius;
            if (i > 0) line(prev, p, color, duration);
            prev = p;
        }
    }
}

void DebugDraw::arrow(glm::vec3 origin, glm::vec3 dir, glm::vec4 color, float duration)
{
    glm::vec3 tip = origin + dir;
    line(origin, tip, color, duration);

    // Small perpendicular cross at the tip
    glm::vec3 up = std::abs(glm::dot(glm::normalize(dir), glm::vec3(0,1,0))) < 0.9f
                   ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
    glm::vec3 right = glm::normalize(glm::cross(glm::normalize(dir), up));
    float hs = glm::length(dir) * 0.1f;
    line(tip - right * hs, tip + right * hs, color, duration);
}

void DebugDraw::axes(glm::vec3 origin, float length, float duration)
{
    arrow(origin, {length, 0, 0}, {1,0,0,1}, duration);
    arrow(origin, {0, length, 0}, {0,1,0,1}, duration);
    arrow(origin, {0, 0, length}, {0,0,1,1}, duration);
}

void DebugDraw::text(glm::vec3 worldPos, std::string label, glm::vec4 color, float duration)
{
    m_texts.push_back({worldPos, std::move(label), color, duration});
}

void DebugDraw::clear()
{
    m_vertices.clear();
    m_persistent.clear();
    m_texts.clear();
}

// ---- Flush (GL lines only — call from doDraw, outside ImGui frame) ---------

void DebugDraw::flush(const glm::mat4& viewProj, glm::vec4 viewport, float dt)
{
    init();

    // Cache for drawUi() which runs in the next doDrawUi() call
    m_lastViewProj = viewProj;
    m_lastViewport = viewport;

    // Add persistent lines (still alive) to this frame's batch
    for (auto& pl : m_persistent)
        addLine(pl.p0, pl.p1, pl.color);

    // Upload and draw line geometry
    if (!m_vertices.empty()) {
        GLint previousProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);

        glUseProgram(m_program);
        glUniformMatrix4fv(glGetUniformLocation(m_program, "viewProj"), 1, GL_FALSE,
                           glm::value_ptr(viewProj));

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     GLsizeiptr(m_vertices.size() * sizeof(Vertex)),
                     m_vertices.data(), GL_DYNAMIC_DRAW);

        GLboolean prevDepthTest;
        glGetBooleanv(GL_DEPTH_TEST, &prevDepthTest);
        glEnable(GL_DEPTH_TEST);

        glDrawArrays(GL_LINES, 0, GLsizei(m_vertices.size()));

        if (!prevDepthTest) glDisable(GL_DEPTH_TEST);
        glBindVertexArray(0);
        glUseProgram(GLuint(previousProgram));

        m_vertices.clear();
    }

    // Age persistent lines
    m_persistent.erase(
        std::remove_if(m_persistent.begin(), m_persistent.end(),
            [dt](PersistentLine& pl){ pl.remainingTime -= dt; return pl.remainingTime <= 0.0f; }),
        m_persistent.end());
}

// ---- DrawUi (ImGui text — call from doDrawUi, inside ImGui frame) ----------

void DebugDraw::drawUi()
{
    if (m_texts.empty()) return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    if (!dl) { m_texts.clear(); return; }

    float vpX = m_lastViewport.x, vpY = m_lastViewport.y;
    float vpW = m_lastViewport.z, vpH = m_lastViewport.w;

    auto projectToScreen = [&](glm::vec3 world) -> glm::vec2 {
        glm::vec4 clip = m_lastViewProj * glm::vec4(world, 1.0f);
        if (std::abs(clip.w) < 1e-6f) return {-9999, -9999};
        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        return {
            vpX + (ndc.x * 0.5f + 0.5f) * vpW,
            vpY + (1.0f - (ndc.y * 0.5f + 0.5f)) * vpH
        };
    };

    auto surviving = std::vector<TextEntry>{};
    for (auto& te : m_texts) {
        auto sc = projectToScreen(te.worldPos);
        if (sc.x > -9000) {
            ImU32 imCol = IM_COL32(
                int(te.color.r * 255), int(te.color.g * 255),
                int(te.color.b * 255), int(te.color.a * 255));
            dl->AddText({sc.x, sc.y}, imCol, te.label.c_str());
        }
        if (te.remainingTime > 0.0f)
            surviving.push_back(te);
    }
    m_texts = std::move(surviving);
}

} // namespace ivf
