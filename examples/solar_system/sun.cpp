#include "sun.h"

using namespace solar;

double constrain(double value, double minValue, double maxValue)
{
    if (value<minValue)
        return minValue;

    if (value>maxValue)
        return maxValue;

    return value;
}

Sun::Sun(double m, double x, double y, double z)
    : m_loc(x, y, z)
	, m_mass(m)
	, m_G(10.0)
{}

solar::Sun::Sun(const Sun &sunCopy)
{
	m_mass = sunCopy.m_mass;
	m_loc = sunCopy.m_loc;
	m_G = sunCopy.m_G;
}

Sun &solar::Sun::operator=(Sun const &other)
{
    if (this != &other)
    {
		m_mass = other.m_mass;
		m_loc = other.m_loc;
		m_G = other.m_G;
	}
	return *this;
}

std::shared_ptr<Sun> solar::Sun::create(double m, double x, double y, double z)
{
    return std::make_shared<Sun>(m, x, y, z);
}

Vec3d Sun::attract(Planet* m) const
{
    Vec3d force = m_loc - m->location();
    double d = force.length();
    d = constrain(d, 5.0, 25.0);
    double strength = (m_G * m_mass * m->mass()) / (d * d);
    return force * strength / d;
}

Vec3d Sun::location()
{
    return m_loc;
}

double Sun::mass() const
{
    return m_mass;
}
