#pragma once

#include <functional>
#include <vector>

#include "base.h"
#include "planet.h"
#include "sun.h"

namespace solar {

typedef std::function<void(size_t idx, double x, double y, double z, double m)> CreatePlanetFunc;
typedef std::function<void(size_t idx, double x, double y, double z, double m)> CreateSunFunc;
typedef std::function<void(size_t idx, double x, double y, double z)> UpdatePlanetPosFunc;
typedef std::function<void()> ClearPlanetsFunc;
typedef std::function<void()> ClearSunsFunc;

class SolarSystem : public Base {
private:
    std::vector<PlanetPtr> m_planets;
    std::vector<SunPtr> m_suns;

    int m_nPlanets;
    int m_nSuns;

    double m_planetRadiusMin;
    double m_planetRadiusMax;
    double m_planetSpeedMin;
    double m_planetSpeedMax;
    double m_planetMassMin;
    double m_planetMassMax;

    bool m_running;

    CreatePlanetFunc m_createPlanetFunc;
    CreateSunFunc m_createSunFunc;
    UpdatePlanetPosFunc m_updatePlanetPosFunc;
    ClearPlanetsFunc m_clearPlanetsFunc;
    ClearSunsFunc m_clearSunsFunc;

public:
    SolarSystem(int n = 100, int m = 2);
    virtual ~SolarSystem();

    static std::shared_ptr<SolarSystem> create(int n = 100, int m = 2);

    void init();
    void clear();
    int planetCount();
    int sunCount();

    void setSize(int planets, int suns);

    Planet *planetAt(int idx);
    Sun *sunAt(int idx);

    void update(double dt);

    void setPlanetCount(int value);
    void setSunCount(int value);

    void setPlanetRadiusMax(double value);
    void setPlanetRadiusMin(double value);
    void setPlanetSpeedMax(double value);
    void setPlanetSpeedMin(double value);
    void setPlanetMassMax(double value);
    void setPlanetMassMin(double value);

    double planetRadiusMax() const;
    double planetRadiusMin() const;
    double planetSpeedMax() const;
    double planetSpeedMin() const;
    double planetMassMax() const;
    double planetMassMin() const;

    bool isRunning() const;
    void run();
    void pause();

    void assignClearPlanetsFunc(ClearPlanetsFunc &clearPlanetsFunc);
    void assignClearSunsFunc(ClearSunsFunc &clearSunsFunc);
    void assignCreatePlanetFunc(CreatePlanetFunc &createPlanetFunc);
    void assignCreateSunFunc(CreateSunFunc &createSunFunc);
    void assignUpdatePlanetPosFunc(UpdatePlanetPosFunc &updatePlanetPosFunc);
};

typedef std::shared_ptr<SolarSystem> SolarSystemPtr;

} // namespace solar
