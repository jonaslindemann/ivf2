#pragma once

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include "solar_system.h"
#include "solar_panel.h"

namespace solar_ui {

class SolarWindow : public ivfui::GLFWSceneWindow {
private:
    ivf::CompositeNodePtr m_scene;
    ivfui::CameraManipulatorPtr m_camManip;

    ivf::MaterialPtr m_planetMaterial;
    ivf::MaterialPtr m_sunMaterial;

    ivf::CompositeNodePtr m_planets;
    ivf::CompositeNodePtr m_suns;
    ivf::LightManagerPtr m_lightMgr;

    solar::SolarSystemPtr m_solarSystem;

    ivfui::PointLightWindowPtr m_pointLightWindow;
    SolarPanelPtr m_solarPanel;

    ivfui::FpsWindowPtr m_fpsWindow;

    solar::ClearSunsFunc m_clearSunsFunc;
    solar::ClearPlanetsFunc m_clearPlanetsFunc;
    solar::CreatePlanetFunc m_createPlanetFunc;
    solar::CreateSunFunc m_createSunFunc;
    solar::UpdatePlanetPosFunc m_updatePlanetPosFunc;

public:
    SolarWindow(int width, int height, std::string title);

    static std::shared_ptr<SolarWindow> create(int width, int height, std::string title);
 
    virtual int onSetup() override;
    virtual void onUpdate() override;

    void onClearPlanets();
    void onClearSuns();
    void onCreatePlanet(size_t idx, double x, double y, double z, double m);
    void onCreateSun(size_t idx, double x, double y, double z, double m);
    void onUpdatePlanetPos(size_t idx, double x, double y, double z);
};

typedef std::shared_ptr<SolarWindow> SolarWindowPtr;

} // namespace solar	