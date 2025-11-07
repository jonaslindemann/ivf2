#include "solar_system.h"
#include "utils.h"
#include "vec3d.h"

#include <cmath>
#include <iostream>

using namespace std;
using namespace solar;

SolarSystem::SolarSystem(int n, int m)
    : Base(), m_nPlanets(n), m_nSuns(m), m_planetRadiusMin(30.0), m_planetRadiusMax(100.0), m_planetSpeedMin(10.0),
      m_planetSpeedMax(20.0), m_planetMassMin(0.1), m_planetMassMax(2.0), m_createPlanetFunc(nullptr),
      m_createSunFunc(nullptr), m_updatePlanetPosFunc(nullptr), m_clearPlanetsFunc(nullptr), m_clearSunsFunc(nullptr),
      m_running(true)
{}

SolarSystem::~SolarSystem()
{
    this->clear();
}

std::shared_ptr<SolarSystem> solar::SolarSystem::create(int n, int m)
{
    return std::make_shared<SolarSystem>(n, m);
}

void SolarSystem::init()
{
    this->clear();

    for (int i = 0; i < m_nPlanets; i++)
    {
        double r = random(m_planetRadiusMin, m_planetRadiusMax);
        double v = random(m_planetSpeedMin, m_planetSpeedMax);
        double alfa = random(0.0, 2 * 3.14159);
        double x = r * cos(alfa);
        double y = r * sin(alfa);
        double vx = -v * sin(alfa);
        double vy = v * cos(alfa);
        auto planet = Planet::create(random(m_planetMassMin, m_planetMassMax), x, 0.0, y);
        planet->setVelocity(vx, 0.0, vy);

        if (m_createPlanetFunc)
            m_createPlanetFunc(i, planet->location().x(), planet->location().y(), planet->location().z(),
                               planet->mass());

        m_planets.push_back(planet);
    }

    for (int i = 0; i < m_nSuns; i++)
    {
        auto sun = Sun::create(random(10, 20), random(-40.0 / 2, 40.0 / 2), 0, random(-40.0 / 2, 40.0 / 2));

        if (m_createSunFunc)
            m_createSunFunc(i, sun->location().x(), sun->location().y(), sun->location().z(), sun->mass());

        m_suns.push_back(sun);
    }
}

void SolarSystem::clear()
{
    if (m_clearPlanetsFunc)
        m_clearPlanetsFunc();

    m_planets.clear();

    if (m_clearSunsFunc)
        m_clearSunsFunc();

    m_suns.clear();
}

void SolarSystem::update(double dt)
{
    if (!m_running)
        return;

    size_t i = 0;

    for (auto &planet : m_planets)
    {
        for (auto &sun : m_suns)
        {
            Vec3d force = sun->attract(planet.get());
            planet->applyForce(force);
        }
        if (m_updatePlanetPosFunc)
            m_updatePlanetPosFunc(i++, planet->location().x(), planet->location().y(), planet->location().z());

        planet->update(dt);
    }
}

void solar::SolarSystem::setPlanetCount(int value)
{
	m_nPlanets = value;
}

void solar::SolarSystem::setSunCount(int value)
{
	m_nSuns = value;
}

int SolarSystem::planetCount()
{
    return m_planets.size();
}

int SolarSystem::sunCount()
{
    return m_suns.size();
}

void SolarSystem::setSize(int planets, int suns)
{
    m_nPlanets = planets;
    m_nSuns = suns;
}

Planet *SolarSystem::planetAt(int idx)
{
    if ((idx >= 0) && (idx < m_planets.size()))
        return m_planets[idx].get();
    else
        return nullptr;
}

Sun *SolarSystem::sunAt(int idx)
{
    if ((idx >= 0) && (idx < m_suns.size()))
        return m_suns[idx].get();
    else
        return nullptr;
}

void SolarSystem::setPlanetRadiusMax(double value)
{
    m_planetRadiusMax = value;
}

void SolarSystem::setPlanetRadiusMin(double value)
{
    m_planetRadiusMin = value;
}

void SolarSystem::setPlanetSpeedMax(double value)
{
    m_planetSpeedMax = value;
}

void SolarSystem::setPlanetSpeedMin(double value)
{
    m_planetSpeedMin = value;
}

void SolarSystem::setPlanetMassMax(double value)
{
    m_planetMassMax = value;
}

void SolarSystem::setPlanetMassMin(double value)
{
    m_planetMassMin = value;
}

double SolarSystem::planetRadiusMax() const
{
    return m_planetRadiusMax;
}

double SolarSystem::planetRadiusMin() const
{
    return m_planetRadiusMin;
}

double SolarSystem::planetSpeedMax() const
{
    return m_planetSpeedMax;
}

double SolarSystem::planetSpeedMin() const
{
    return m_planetSpeedMin;
}

double SolarSystem::planetMassMax() const
{
    return m_planetMassMax;
}

double SolarSystem::planetMassMin() const
{
    return m_planetMassMin;
}

bool solar::SolarSystem::isRunning() const
{
    return m_running;
}

void solar::SolarSystem::run()
{
    m_running = true;
}

void solar::SolarSystem::pause()
{
    m_running = false;
}

void solar::SolarSystem::assignClearPlanetsFunc(ClearPlanetsFunc &clearPlanetsFunc)
{
    m_clearPlanetsFunc = clearPlanetsFunc;
}

void solar::SolarSystem::assignClearSunsFunc(ClearSunsFunc &clearSunsFunc)
{
    m_clearSunsFunc = clearSunsFunc;
}

void solar::SolarSystem::assignCreatePlanetFunc(CreatePlanetFunc &createPlanetFunc)
{
    m_createPlanetFunc = createPlanetFunc;
}

void solar::SolarSystem::assignCreateSunFunc(CreateSunFunc &createSunFunc)
{
    m_createSunFunc = createSunFunc;
}

void solar::SolarSystem::assignUpdatePlanetPosFunc(UpdatePlanetPosFunc &updatePlanetPosFunc)
{
    m_updatePlanetPosFunc = updatePlanetPosFunc;
}
