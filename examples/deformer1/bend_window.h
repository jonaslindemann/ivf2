#pragma once

#include <ivfui/ui.h>

namespace ivfui {

class BendWindow : public UiWindow {
private:
    float m_curvature;
    float m_startDistance;
    float m_endDistance;

    float m_center[3] = {0.0f, 0.0f, 0.0f}; // Center of twist (default: origin)
    float m_axis[3] = {0.0f, 1.0f, 0.0f};   // Twist axis (default: Y-axis)

public:
    BendWindow();

    static std::shared_ptr<BendWindow> create();

    void doDraw() override;

    float curvature() const;
    void setCurvature(float curvature);

    float startDistance() const;
    void setStartDistance(float startDistance);

    float endDistance() const;
    void setEndDistance(float endDistance);

    const float *center() const;
    void setCenter(const float center[3]);

    const float *axis() const;
    void setAxis(const float axis[3]);
};

typedef std::shared_ptr<BendWindow> BendWindowPtr;

} // namespace ivfui
