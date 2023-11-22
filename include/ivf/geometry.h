#pragma once
#include <ivf/glbase.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

//
// Based on code by Song ho Ahn
//
// http://www.songho.ca/opengl/gl_cylinder.html#example_pipe
//

namespace ivf {

	class Line {
	private:
		glm::vec3 m_direction;
		glm::vec3 m_point;
	public:
		Line() : m_direction(glm::vec3()), m_point(glm::vec3()) {}
		Line(const glm::vec3& v, const glm::vec3& p) : m_direction(v), m_point(p) {}    
		~Line() {};

        void set(const glm::vec3& v, const glm::vec3& p);               // from 3D
        void setPoint(glm::vec3& p) { m_point = p; }
        void setDirection(const glm::vec3& v) { m_direction = v; }
        const glm::vec3& point() const { return m_point; }
        const glm::vec3& direction() const { return m_direction; }

        // find intersect point with other line
        glm::vec3 intersect(const Line& line);
        bool isIntersected(const Line& line);

	};

	class Plane {
	private:
		glm::vec3 m_normal;     // normal vector of a plane
		float m_d;            // coefficient of constant term: d = -(a*x0 + b*y0 + c*z0)
		float m_normalLength; // length of normal vector
		float m_distance;     // distance from origin to plane
	public:
        Plane();
        Plane(float a, float b, float c, float d);          // 4 coeff of plane equation
        Plane(const glm::vec3& normal, const glm::vec3& point); // a point on the plane and normal vector
        ~Plane() {}

        // setters/getters
        void set(float a, float b, float c, float d);
        void set(const glm::vec3& normal, const glm::vec3& point);  // set with  a point on the plane and normal
        const glm::vec3& normal() const { return m_normal; }
        float d() const { return m_d; }                        // return 4th coefficient
        float normalLength() const { return m_normalLength; }  // return length of normal
        float distance() const { return m_distance; };         // return distance from the origin
        float distance(const glm::vec3& point);                // return distance from the point

        // convert plane equation with unit normal vector
        void normalize();

        // for intersection
        glm::vec3 intersect(const Line& line) const;              // intersect with a line
        Line intersect(const Plane& plane) const;               // intersect with another plane
        bool isIntersected(const Line& line) const;
        bool isIntersected(const Plane& plane) const;
	};

    glm::mat4 lookAt(const glm::vec3& target);

}


