#pragma once

#include <ivfui/ui.h>

namespace ivfui {

class TwistWindow : public UiWindow {
private:
    float m_angle{0.0f};                    // Twist angle in radians
    float m_falloff{1.0};                   // Distance falloff factor
    float m_startDistance{0.0};             // Distance where twist starts
    float m_endDistance{1.0};               // Distance where twist ends
    bool m_wireframe{false};                // Wireframe mode
    float m_angleIncrement{0.5f};           // Angle increment for animation
    float m_center[3] = {0.0f, 0.0f, 0.0f}; // Center of twist (default: origin)
    float m_axis[3] = {0.0f, 1.0f, 0.0f};   // Twist axis (default: Y-axis)

public:
    TwistWindow();

    static std::shared_ptr<TwistWindow> create();

    void doDraw() override;

    float angle() const;
    void setAngle(float angle);

    float falloff() const;
    void setFalloff(float falloff);

    float startDistance() const;
    void setStartDistance(float startDistance);

    float endDistance() const;
    void setEndDistance(float endDistance);

    bool wireframe() const;
    void setWireframe(bool wireframe);

    const float *center() const;
    void setCenter(const float center[3]);

    const float *axis() const;
    void setAxis(const float axis[3]);
};

typedef std::shared_ptr<TwistWindow> TwistWindowPtr;

} // namespace ivfui
