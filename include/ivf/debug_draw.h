#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

namespace ivf {

/**
 * @brief Immediate-mode debug drawing singleton for rapid prototyping.
 *
 * All draw calls are ephemeral: they appear for a single frame (or `duration` seconds)
 * and are discarded automatically. Geometry is batched into one GPU draw call per flush.
 *
 * GLFWSceneWindow calls flush() automatically each frame after scene rendering.
 *
 * Usage:
 * @code
 * auto& dd = *DebugDraw::instance();
 * dd.line({0,0,0}, {1,1,0});
 * dd.box({0,1,0}, {0.5,0.5,0.5});
 * dd.arrow({0,0,0}, {0,1,0}, {1,0,0,1});
 * dd.text({2,0,0}, "hello");   // projected to screen via ImGui
 * @endcode
 */
class DebugDraw {
private:
    struct Vertex {
        glm::vec3 pos;
        glm::vec4 color;
    };

    struct TextEntry {
        glm::vec3 worldPos;
        std::string label;
        glm::vec4  color;
        float remainingTime;
    };

    struct PersistentLine {
        glm::vec3 p0, p1;
        glm::vec4 color;
        float remainingTime;
    };

    std::vector<Vertex> m_vertices;       // one-frame line vertices (2 per segment)
    std::vector<PersistentLine> m_persistent; // duration-based lines
    std::vector<TextEntry> m_texts;

    GLuint m_vao{0};
    GLuint m_vbo{0};
    GLuint m_program{0};
    bool   m_initialized{false};

    glm::mat4 m_lastViewProj{1.0f};
    glm::vec4 m_lastViewport{0,0,1,1};

    static DebugDraw* m_instance;

    DebugDraw() = default;

    void init();
    void addLine(glm::vec3 p0, glm::vec3 p1, glm::vec4 color);

public:
    static DebugDraw* instance()
    {
        if (!m_instance)
            m_instance = new DebugDraw();
        return m_instance;
    }

    // ---- Primitives (one-frame unless duration > 0) ----

    /** Draw a line segment between p0 and p1. */
    void line(glm::vec3 p0, glm::vec3 p1, glm::vec4 color = {1,1,0,1}, float duration = 0.0f);

    /** Draw a wireframe axis-aligned box centered at `center` with half-extents `halfSize`. */
    void box(glm::vec3 center, glm::vec3 halfSize, glm::vec4 color = {1,1,0,1}, float duration = 0.0f);

    /** Draw a wireframe sphere approximation (latitude/longitude lines). */
    void sphere(glm::vec3 center, float radius, glm::vec4 color = {0,1,1,1}, float duration = 0.0f, int segments = 16);

    /** Draw an arrow: line from origin along dir, with a small arrowhead cross. */
    void arrow(glm::vec3 origin, glm::vec3 dir, glm::vec4 color = {1,0,0,1}, float duration = 0.0f);

    /** Draw world-space axes at a position. */
    void axes(glm::vec3 origin, float length = 1.0f, float duration = 0.0f);

    /**
     * Draw a text label projected from worldPos to screen space.
     * Rendered via ImGui ForegroundDrawList — call after flush() or during doDrawUi().
     */
    void text(glm::vec3 worldPos, std::string label, glm::vec4 color = {1,1,1,1}, float duration = 0.0f);

    /**
     * Flush GL line geometry to GPU and draw. Call from doDraw() (outside ImGui frame).
     * Caches viewProj/viewport for the subsequent drawUi() call.
     * @param viewProj  Combined view-projection matrix (proj * view).
     * @param viewport  {x, y, width, height} of the current viewport in pixels.
     * @param dt        Frame delta time in seconds, for persistent primitives.
     */
    void flush(const glm::mat4& viewProj, glm::vec4 viewport, float dt = 0.0f);

    /**
     * Render screen-space text labels via ImGui ForegroundDrawList.
     * Must be called from doDrawUi() while an ImGui frame is active.
     * Uses the viewProj/viewport cached by the previous flush() call.
     */
    void drawUi();

    /** Remove all one-frame and persistent primitives immediately. */
    void clear();
};

} // namespace ivf
