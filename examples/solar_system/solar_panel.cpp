#include "solar_panel.h"

using namespace solar_ui;

solar_ui::SolarPanel::SolarPanel(std::string caption)
    : ivfui::UiWindow(caption), m_solarSystem(nullptr), m_nPlanets(0), m_nSuns(0), m_planetMassMin(0.0),
      m_planetMassMax(0.0), m_planetRadiusMin(0.0), m_planetRadiusMax(0.0), m_planetSpeedMin(0.0), m_planetSpeedMax(0.0)
{}

solar_ui::SolarPanel::~SolarPanel()
{}

std::shared_ptr<SolarPanel> solar_ui::SolarPanel::create(std::string caption)
{
    return std::make_shared<SolarPanel>(caption);
}

void solar_ui::SolarPanel::setSolarSystem(solar::SolarSystemPtr solarSystem)
{
    m_solarSystem = solarSystem;
}

void solar_ui::SolarPanel::update()
{
    if (m_solarSystem == nullptr)
        return;

    m_nPlanets = m_solarSystem->planetCount();
    m_nSuns = m_solarSystem->sunCount();

    m_planetMassMax = m_solarSystem->planetMassMax();
    m_planetMassMin = m_solarSystem->planetMassMin();
    m_planetRadiusMax = m_solarSystem->planetRadiusMax();
    m_planetRadiusMin = m_solarSystem->planetRadiusMin();
    m_planetSpeedMin = m_solarSystem->planetSpeedMin();
    m_planetSpeedMax = m_solarSystem->planetSpeedMax();
}

void solar_ui::SolarPanel::doDraw()
{
    bool dirty = false;

    if (ImGui::SliderInt("Planets", &m_nPlanets, 2, 1000))
    {
        m_solarSystem->setPlanetCount(m_nPlanets);
        dirty = true;
    }
    if (ImGui::SliderInt("Suns", &m_nSuns, 1, 10))
    {
        m_solarSystem->setSunCount(m_nSuns);
        dirty = true;
    }

    if (ImGui::SliderFloat("Planet Mass Min", &m_planetMassMin, 0.0f, 100.0f))
    {
        m_solarSystem->setPlanetMassMin(m_planetMassMin);
        dirty = true;
    }

    if (ImGui::SliderFloat("Planet Mass Max", &m_planetMassMax, 0.0f, 100.0f))
    {
        m_solarSystem->setPlanetMassMax(m_planetMassMax);
        dirty = true;
    }

    if (ImGui::SliderFloat("Planet Radius Min", &m_planetRadiusMin, 0.0f, 100.0f))
    {
		m_solarSystem->setPlanetRadiusMin(m_planetRadiusMin);
		dirty = true;
	}

    if (ImGui::SliderFloat("Planet Radius Max", &m_planetRadiusMax, 0.0f, 100.0f))
    {
        m_solarSystem->setPlanetRadiusMax(m_planetRadiusMax);
        dirty = true;
    }

    if (ImGui::SliderFloat("Planet Speed Min", &m_planetSpeedMin, 0.0f, 100.0f))
    {
		m_solarSystem->setPlanetSpeedMin(m_planetSpeedMin);
		dirty = true;
	}

    if (ImGui::SliderFloat("Planet Speed Max", &m_planetSpeedMax, 0.0f, 100.0f))
    {
		m_solarSystem->setPlanetSpeedMax(m_planetSpeedMax);
		dirty = true;
	}

    if (dirty)
        m_solarSystem->init();
}
