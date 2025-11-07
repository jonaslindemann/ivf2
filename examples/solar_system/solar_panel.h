#pragma once

#include <memory>

#include <ivfui/ui_window.h>
#include <ivf/material.h>

#include "solar_system.h"

namespace solar_ui {

class SolarPanel : public ivfui::UiWindow {
private:
	solar::SolarSystemPtr m_solarSystem;

    ivf::MaterialPtr m_planetMaterial;
    ivf::MaterialPtr m_sunMaterial;
    ivf::MaterialPtr m_tracerMaterial;

    int m_nPlanets;
    int m_nSuns;

    float m_planetRadiusMin;
    float m_planetRadiusMax;
    float m_planetSpeedMin;
    float m_planetSpeedMax;
    float m_planetMassMin;
    float m_planetMassMax;

    float m_planetColor[3];
    float m_sunColor[3];
    float m_tracerColor[3];

    bool m_showTracers{true};
    int m_nTracersPerPlanet{100};

public:
	SolarPanel(std::string caption);
	virtual ~SolarPanel();

	static std::shared_ptr<SolarPanel> create(std::string caption);

	void setSolarSystem(solar::SolarSystemPtr solarSystem);
    void setPlanetMaterial(ivf::MaterialPtr material);
    void setSunMaterial(ivf::MaterialPtr material);
    void setTracerMaterial(ivf::MaterialPtr material);

    bool showTracers() const;
    int tracersPerPlanet() const;

    void setShowTracers(bool flag);
    void setTracersPerPlanet(int n);

    void update();

    void doDraw() override;

};

typedef std::shared_ptr<SolarPanel> SolarPanelPtr;

} // namespace solar_ui
