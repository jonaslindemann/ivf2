#include "solar_panel.h"

#include <format>

using namespace solar_ui;

solar_ui::SolarPanel::SolarPanel(std::string caption)
    : ivfui::UiWindow(caption), m_solarSystem(nullptr), m_nPlanets(0), m_nSuns(0), m_planetMassMin(0.0),
      m_planetMassMax(0.0), m_planetRadiusMin(0.0), m_planetRadiusMax(0.0), m_planetSpeedMin(0.0),
      m_planetSpeedMax(0.0), m_planetMaterial(nullptr), m_sunMaterial(nullptr), m_tracerMaterial(nullptr),
      m_planetColor{1.0f, 1.0f, 1.0f}, m_sunColor{1.0f, 1.0f, 1.0f}, m_tracerColor{0.0f, 1.0f, 1.0f},
      m_showTracers(true), m_nTracersPerPlanet(100)
{
    this->update();
}

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

void solar_ui::SolarPanel::setPlanetMaterial(ivf::MaterialPtr material)
{
    m_planetMaterial = material;
}

void solar_ui::SolarPanel::setSunMaterial(ivf::MaterialPtr material)
{
    m_sunMaterial = material;
}

void solar_ui::SolarPanel::setTracerMaterial(ivf::MaterialPtr material)
{
    m_tracerMaterial = material;
}

bool solar_ui::SolarPanel::showTracers() const
{
    return m_showTracers;
}

int solar_ui::SolarPanel::tracersPerPlanet() const
{
    return m_nTracersPerPlanet;
}

void solar_ui::SolarPanel::setShowTracers(bool flag)
{
    m_showTracers = flag;
}

void solar_ui::SolarPanel::setTracersPerPlanet(int n)
{
    m_nTracersPerPlanet = n;
}

void solar_ui::SolarPanel::update()
{
    if (m_solarSystem == nullptr)
        return;

    m_nPlanets = m_solarSystem->planetCount();
    m_nSuns = m_solarSystem->sunCount();
    m_showTracers = true;
    m_nTracersPerPlanet = 100;

    m_planetMassMax = m_solarSystem->planetMassMax();
    m_planetMassMin = m_solarSystem->planetMassMin();
    m_planetRadiusMax = m_solarSystem->planetRadiusMax();
    m_planetRadiusMin = m_solarSystem->planetRadiusMin();
    m_planetSpeedMin = m_solarSystem->planetSpeedMin();
    m_planetSpeedMax = m_solarSystem->planetSpeedMax();

    m_planetColor[0] = m_planetMaterial->diffuseColor().r;
    m_planetColor[1] = m_planetMaterial->diffuseColor().g;
    m_planetColor[2] = m_planetMaterial->diffuseColor().b;

    m_sunColor[0] = m_sunMaterial->diffuseColor().r;
    m_sunColor[1] = m_sunMaterial->diffuseColor().g;
    m_sunColor[2] = m_sunMaterial->diffuseColor().b;

    m_tracerColor[0] = m_tracerMaterial->diffuseColor().r;
    m_tracerColor[1] = m_tracerMaterial->diffuseColor().g;
    m_tracerColor[2] = m_tracerMaterial->diffuseColor().b;


}

void solar_ui::SolarPanel::doDraw()
{
    bool dirty = false;

    ImGui::SeparatorText("Simulation controls");

    if (ImGui::Button(m_solarSystem->isRunning() ? "Pause" : "Run"))
    {
        if (m_solarSystem->isRunning())
            m_solarSystem->pause();
        else
            m_solarSystem->run();
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset"))
    {
        m_solarSystem->pause();
        m_solarSystem->init();
        m_solarSystem->run();
    }

    ImGui::SeparatorText("Initial settings");

    if (ImGui::SliderInt("Planets", &m_nPlanets, 2, 1000))
        m_solarSystem->setPlanetCount(m_nPlanets);

    if (ImGui::SliderInt("Suns", &m_nSuns, 1, 10))
        m_solarSystem->setSunCount(m_nSuns);

    if (ImGui::SliderFloat("Planet Mass Min", &m_planetMassMin, 0.0f, 100.0f))
        m_solarSystem->setPlanetMassMin(m_planetMassMin);

    if (ImGui::SliderFloat("Planet Mass Max", &m_planetMassMax, 0.0f, 100.0f))
        m_solarSystem->setPlanetMassMax(m_planetMassMax);

    if (ImGui::SliderFloat("Planet Radius Min", &m_planetRadiusMin, 0.0f, 100.0f))
		m_solarSystem->setPlanetRadiusMin(m_planetRadiusMin);

    if (ImGui::SliderFloat("Planet Radius Max", &m_planetRadiusMax, 0.0f, 100.0f))
        m_solarSystem->setPlanetRadiusMax(m_planetRadiusMax);

    if (ImGui::SliderFloat("Planet Speed Min", &m_planetSpeedMin, 0.0f, 100.0f))
		m_solarSystem->setPlanetSpeedMin(m_planetSpeedMin);

    if (ImGui::SliderFloat("Planet Speed Max", &m_planetSpeedMax, 0.0f, 100.0f))
		m_solarSystem->setPlanetSpeedMax(m_planetSpeedMax);

    if (ImGui::SliderInt("Tracers per Planet", &m_nTracersPerPlanet, 0, 500))
        m_nTracersPerPlanet = m_nTracersPerPlanet;

    ImGui::SeparatorText("Dynamic properties");

    for (auto i = 0; i < m_solarSystem->sunCount(); ++i)
    {
        auto sun = m_solarSystem->sunAt(i);
        std::string label = std::format("Sun {}", i + 1);
        ImGui::DragScalar(label.c_str(), ImGuiDataType_Double, sun->massPtr(), 0.1f, nullptr, nullptr, "%.2f");
    }

    ImGui::SeparatorText("Colors");

    if (ImGui::ColorEdit3("Planet Color", m_planetColor, ImGuiColorEditFlags_None))
    {
        if (m_planetMaterial)
            m_planetMaterial->setDiffuseColor(glm::vec4(m_planetColor[0], m_planetColor[1], m_planetColor[2], 1.0f));
    }

    if (ImGui::ColorEdit3("Sun Color", m_sunColor, ImGuiColorEditFlags_None))
    {
        if (m_sunMaterial)
            m_sunMaterial->setDiffuseColor(glm::vec4(m_sunColor[0], m_sunColor[1], m_sunColor[2], 1.0f));
    }

    if (ImGui::ColorEdit3("Tracer Color", m_tracerColor, ImGuiColorEditFlags_None))
    {
        if (m_tracerMaterial)
            m_tracerMaterial->setDiffuseColor(glm::vec4(m_tracerColor[0], m_tracerColor[1], m_tracerColor[2], 1.0f));
    }

    ImGui::Checkbox("Show Tracers", &m_showTracers);
}
