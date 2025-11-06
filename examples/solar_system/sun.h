#pragma once

#include <memory>

#include "base.h"
#include "vec3d.h"
#include "planet.h"

namespace solar {

class Sun : public Base {
private:
    double m_mass{1.0};
    Vec3d m_loc{0.0, 0.0, 0.0};
    double m_G{10.0};

public:
    Sun() = default;
    Sun(double m, double x, double y, double z);
    Sun(const Sun &sunCopy);
    Sun &operator=(Sun const &other);


    static std::shared_ptr<Sun> create(double m, double x, double y, double z);

    Vec3d attract(Planet* m) const;
    Vec3d location();

    double mass() const;

};

typedef std::shared_ptr<Sun> SunPtr;

} // namespace solar