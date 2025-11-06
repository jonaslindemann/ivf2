#pragma once

#include <memory>

#include <ivfui/ui_window.h>

#include "solar_system.h"

namespace solar_ui {

class SolarPanel : public ivfui::UiWindow {
private:
	solar::SolarSystemPtr m_solarSystem;

    int m_nPlanets;
    int m_nSuns;

    float m_planetRadiusMin;
    float m_planetRadiusMax;
    float m_planetSpeedMin;
    float m_planetSpeedMax;
    float m_planetMassMin;
    float m_planetMassMax;

public:
	SolarPanel(std::string caption);
	virtual ~SolarPanel();

	static std::shared_ptr<SolarPanel> create(std::string caption);

	void setSolarSystem(solar::SolarSystemPtr solarSystem);

    void update();

    void doDraw() override;

};

typedef std::shared_ptr<SolarPanel> SolarPanelPtr;

} // namespace solar_ui
