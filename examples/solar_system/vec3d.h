#pragma once

/**
 * Simple 3 component vector class
 *
 * Vec3d implements a 3 component vector with standard
 * operators and methods. It should also be used instead
 * of the cold IvfVector class.
 * @author Jonas Lindemann
 */
class Vec3d {
private:
	double m_vector[3];
public:
	double z();
	double y();
	double x();
	/** Class constructor. */
	Vec3d();
    
	/** Class constructor. */
	Vec3d(double vx, double vy, double vz);
    
	/** Class assignment constructor. */
	Vec3d(const Vec3d& v);
    
	/** Class destructor. */
	virtual ~Vec3d();
    
	/**
	 * Rotate vector around an axis.
	 *
	 * @param axis is the rotation axis
	 * @param angle is the amount of rotation around the axis.
	 */
	void rotate(Vec3d& axis, double angle);
    
	/** Normalizes the vector. Length = 1.0.*/
	void normalize();
    
	/** Returns the vector length. */
	double length();
    
	/** Negate vector. */
	void negate();
    
	/** Add to vector. */
	void add(double dx, double dy, double dz);
    
	/**
	 * Set vector from to points/vectors.
	 *
	 * @param pos is the starting point of the vector.
	 * @param target is the endpoint of the vector.
	 */
	void setFromPoints(Vec3d& pos, Vec3d& target);
    
	/** Return a pointer to vector components */
	double* getComponents();
    
	/** Set components of vector. */
	void setComponents(double vx, double vy, double vz);
	/** Set components of vector. */
	void setComponents(const double *v);
	/** Get vector components. */
	void getComponents(double &vx, double &vy, double &vz);
	/** Get vector components. */
	void getComponents(double* v);
    
	/** Set components of vector. */
	void setComponents(int x, int y);
    
	/** Set components of vector. */
	void setComponents(const int* pos);
    
	/** Calculate euler angles of vector. */
	void getEulerAngles(double &pitch, double &heading);
    
    
	/** Set z-component of vector */
	void setZ(double value);
	/** Set y-component of vector */
	void setY(double value);
	/** Set x-component of vector */
	void setX(double value);
        
	/** Vector addition operator. */
	Vec3d& operator+=(Vec3d a);
	/** Vector subtraction operator. */
	Vec3d& operator-=(Vec3d a);
	/** Assignment operator. */
	Vec3d& operator=(Vec3d a);
	/** Index operator. */
	double operator[](const int idx);
    
};

/** Vector crossproduct operator. */
Vec3d operator*(Vec3d a, Vec3d b);

/** Vector scalar multiplication operator. */
Vec3d operator*(Vec3d a, double b);

/** Vector scalar multiplication operator. */
Vec3d operator/(Vec3d a, double b);

/** Vector scalar multiplication operator. */
Vec3d operator*(double a, Vec3d b);

/** Vector addition operator. */
Vec3d operator+(Vec3d a, Vec3d b);

/** Vector subtraction operator. */
Vec3d operator-(Vec3d a, Vec3d b);
