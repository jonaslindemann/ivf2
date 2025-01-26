#pragma once

#include <ivfui/ui_window.h>

#include <ivfui/camera_manipulator.h>

namespace ivfui {

class CameraWindow : public ivfui::UiWindow {
private:
    CameraManipulatorPtr m_cameraManipulator;

    float m_cameraTarget[3];
    float m_newCameraTarget[3];
    float m_cameraPosition[3];
    float m_newCameraPosition[3];

    float m_fov;
    float m_newFov;
    float m_nearZ;
    float m_newNearZ;
    float m_farZ;
    float m_newFarZ;

    bool m_isDirty;

    void updateControls();
    void updateCamera();

public:
    CameraWindow(CameraManipulatorPtr camManip, std::string caption);

    static std::shared_ptr<CameraWindow> create(CameraManipulatorPtr camManip, std::string caption);

    void doDraw() override;
    void doUpdate() override;

    bool isDirty();
};

typedef std::shared_ptr<CameraWindow> CameraWindowPtr;
} // namespace ivfui
