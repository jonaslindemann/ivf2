#pragma once

/**
 * @file time_controller.h
 * @brief Declares the TimeController singleton for global scaled/pausable time in the ivf library.
 */

#include <memory>

namespace ivf {

/**
 * @class TimeController
 * @brief Singleton that maintains a scaled, pausable time stream independent of the wall clock.
 *
 * GLFWSceneWindow feeds real frame deltas into update() each frame. User code reads
 * elapsed() and delta() instead of elapsedTime()/frameTime() to participate in
 * time scaling and pause. The wall-clock methods on GLFWWindow remain unchanged.
 *
 * Typical use:
 * @code
 * void onUpdate() {
 *     double t  = ivf::TimeController::instance()->elapsed();
 *     double dt = ivf::TimeController::instance()->delta();
 *     m_wave->setTime(t);
 *     m_particles->update(dt);
 * }
 * void onKey(int key, int, int action, int) {
 *     if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
 *         ivf::TimeController::instance()->togglePause();
 * }
 * @endcode
 */
class TimeController {
private:
    static TimeController *m_instance;

    double m_elapsed{0.0};
    double m_delta{0.0};
    float m_scale{1.0f};
    bool m_paused{false};

    TimeController() = default;

public:
    static TimeController *instance()
    {
        if (!m_instance)
            m_instance = new TimeController();
        return m_instance;
    }

    /** Feed the real wall-clock frame delta each frame (called by GLFWSceneWindow). */
    void update(double realDelta)
    {
        if (m_paused) {
            m_delta = 0.0;
        } else {
            m_delta = realDelta * static_cast<double>(m_scale);
            m_elapsed += m_delta;
        }
    }

    /** Set the time scale. 0 = frozen, 0.5 = half speed, 1 = normal, 2 = double speed. */
    void setScale(float scale) { m_scale = scale; }
    float scale() const { return m_scale; }

    void pause() { m_paused = true; }
    void resume() { m_paused = false; }
    void togglePause() { m_paused = !m_paused; }
    bool isPaused() const { return m_paused; }

    /** Accumulated scaled time in seconds. */
    double elapsed() const { return m_elapsed; }

    /** Scaled delta for the last frame (0 when paused). */
    double delta() const { return m_delta; }

    /** Reset elapsed to 0. Scale and pause state are unaffected. */
    void reset() { m_elapsed = 0.0; m_delta = 0.0; }
};

} // namespace ivf
