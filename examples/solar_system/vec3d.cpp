#include "vec3d.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

Vec3d::Vec3d()
{
	setComponents(0.0, 0.0, 0.0);
}

Vec3d::Vec3d(double vx, double vy, double vz)
{
	m_vector[0] = vx;
	m_vector[1] = vy;
	m_vector[2] = vz;
}

Vec3d::Vec3d(const Vec3d& v)
{
	m_vector[0] = v.m_vector[0];
	m_vector[1] = v.m_vector[1];
	m_vector[2] = v.m_vector[2];
}

Vec3d::~Vec3d()
{
    
}

double Vec3d::x()
{
    return m_vector[0];
}

double Vec3d::y()
{
    return m_vector[1];
}

double Vec3d::z()
{
    return m_vector[2];
}

void Vec3d::setComponents(double vx, double vy, double vz)
{
	m_vector[0] = vx;
	m_vector[1] = vy;
	m_vector[2] = vz;
}

void Vec3d::setComponents(const double *v)
{
	m_vector[0] = v[0];
	m_vector[1] = v[1];
	m_vector[2] = v[2];
}

void Vec3d::setComponents(int x, int y)
{
	m_vector[0] = (double)x;
	m_vector[1] = (double)y;
}

void Vec3d::setComponents(const int* pos)
{
	m_vector[0] = (double)pos[0];
	m_vector[1] = (double)pos[1];
}

void Vec3d::getComponents(double &vx, double &vy, double &vz)
{
	vx = m_vector[0];
	vy = m_vector[1];
	vz = m_vector[2];
}

Vec3d& Vec3d::operator+=(Vec3d a)
{
	m_vector[0] += a.m_vector[0];
	m_vector[1] += a.m_vector[1];
	m_vector[2] += a.m_vector[2];
	return *this;
}

Vec3d& Vec3d::operator=(Vec3d a)
{
	m_vector[0] = a.m_vector[0];
	m_vector[1] = a.m_vector[1];
	m_vector[2] = a.m_vector[2];
	return *this;
}


Vec3d& Vec3d::operator-=(Vec3d a)
{
	m_vector[0] -= a.m_vector[0];
	m_vector[1] -= a.m_vector[1];
	m_vector[2] -= a.m_vector[2];
	return *this;
}

Vec3d operator+(Vec3d a, Vec3d b)
{
	Vec3d r = a;
	return r += b;
}

Vec3d operator-(Vec3d a, Vec3d b)
{
	Vec3d r = a;
	return r -= b;
}

double Vec3d::operator[](const int idx)
{
	if ((idx>=0)&&(idx<3))
		return m_vector[idx];
	else
		return 0.0;
}

Vec3d operator*(Vec3d a, Vec3d b)
{
    
	double c1, c2, c3;
    
	c1 = a[1] * b[2] - a[2] * b[1];
	c2 = a[2] * b[0] - a[0] * b[2];
	c3 = a[0] * b[1] - a[1] * b[0];
	
	Vec3d r(c1, c2, c3);
    
	return r;
}

Vec3d operator*(Vec3d a, double b)
{
	Vec3d r(a[0]*b, a[1]*b, a[2]*b);
    
	return r;
}

Vec3d operator/(Vec3d a, double b)
{
    Vec3d r(a[0]/b, a[1]/b, a[2]/b);

    return r;
}

Vec3d operator*(double a, Vec3d b)
{
	Vec3d r(b[0]*a, b[1]*a, b[2]*a);
    
	return r;
}

void Vec3d::getComponents(double *v)
{
	v[0] = m_vector[0];
	v[1] = m_vector[1];
	v[2] = m_vector[2];
}

double Vec3d::length()
{
	return sqrt(pow(m_vector[0],2) + pow(m_vector[1],2) + pow(m_vector[2],2));
}

void Vec3d::normalize()
{
	double quote = 1.0/length();
    
	m_vector[0] = m_vector[0] * quote;
	m_vector[1] = m_vector[1] * quote;
	m_vector[2] = m_vector[2] * quote;
}


void Vec3d::rotate(Vec3d &axis, double angle)
{
	double cost = cos(angle*2*M_PI/360.0);
	double sint = sin(angle*2*M_PI/360.0);
    
	double rv[3];
    
	rv[0] =
    (cost + (1.0-cost)*pow(axis[0],2))        * m_vector[0] +
    ((1.0-cost)*axis[0]*axis[1]-axis[2]*sint) * m_vector[1] +
    ((1.0-cost)*axis[0]*axis[2]+axis[1]*sint) * m_vector[2];
    
	rv[1] =
    ((1.0-cost)*axis[0]*axis[1]+axis[2]*sint) * m_vector[0] +
    (cost + (1.0-cost)*pow(axis[1],2))        * m_vector[1] +
    ((1.0-cost)*axis[1]*axis[2]-axis[0]*sint) * m_vector[2];
    
	rv[2] =
    ((1.0-cost)*axis[0]*axis[2]-axis[1]*sint) * m_vector[0] +
    ((1.0-cost)*axis[1]*axis[2]+axis[0]*sint) * m_vector[1] +
    (cost+(1.0-cost)*pow(axis[2],2))          * m_vector[2];
    
	m_vector[0] = rv[0];
	m_vector[1] = rv[1];
	m_vector[2] = rv[2];
}

void Vec3d::getEulerAngles(double &pitch, double &heading)
{
	heading = atan2(m_vector[0], m_vector[2])*180.0/M_PI;
	double t = sqrt(pow(m_vector[0],2)+pow(m_vector[2],2));
	pitch = atan2(m_vector[1], t)*180.0/M_PI;
}

void Vec3d::negate()
{
	m_vector[0] = - m_vector[0];
	m_vector[1] = - m_vector[1];
	m_vector[2] = - m_vector[2];
}


void Vec3d::setX(double value)
{
	m_vector[0] = value;
}

void Vec3d::setY(double value)
{
	m_vector[1] = value;
}

void Vec3d::setZ(double value)
{
	m_vector[2] = value;
}

void Vec3d::setFromPoints(Vec3d &pos, Vec3d &target)
{
	Vec3d r;
	r = target - pos;
	r.getComponents(m_vector);
}

void Vec3d::add(double dx, double dy, double dz)
{
	m_vector[0] += dx;
	m_vector[1] += dy;
	m_vector[2] += dz;
}

double* Vec3d::getComponents()
{
	return &m_vector[0];
}
