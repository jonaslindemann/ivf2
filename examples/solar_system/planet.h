#pragma once

#include <memory>

#include "base.h"
#include "vec3d.h"

namespace solar {

class Planet : public Base {
protected:
    Vec3d m_loc{0.0, 0.0, 0.0};
    Vec3d m_vel{0.0, 0.0, 0.0};
    Vec3d m_acc{0.0, 0.0, 0.0};
    double m_mass{1.0};

public:
    Planet(const Planet& planetCopy);
    Planet(double m, double x, double y, double z);
    Planet &operator=(Planet const &other);

    static std::shared_ptr<Planet> create(double m, double x, double y, double z);

    void applyForce(const Vec3d& force);
    void update(double dt);

    void setVelocity(double x, double y, double z);

    Vec3d location();
    double mass() const;

};

typedef std::shared_ptr<Planet> PlanetPtr;

} // namespace solar
