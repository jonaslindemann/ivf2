#include <ivf/geometry.h>

void ivf::Line::set(const glm::vec3& v, const glm::vec3& p)
{
    m_direction = v;
    m_point = p;
}

glm::vec3 ivf::Line::intersect(const Line& line)
{
    const glm::vec3 v2 = line.direction();
    const glm::vec3 p2 = line.point();

    glm::vec3 result = glm::vec3(NAN, NAN, NAN);    // default with NaN

    // find v3 = (p2 - p1) x V2
    glm::vec3 v3 = glm::cross(p2 - m_point, v2); //glm::vec3 v3 = (p2 - m_point).cross(v2);

    // find v4 = V1 x V2
    glm::vec3 v4 = glm::cross(m_direction, v2);  //Vector3 v4 = direction.cross(v2);

    // find (V1xV2) . (V1xV2)
    float dot = glm::dot(v4, v4);  //float dot = v4.dot(v4);

    // if both V1 and V2 are same direction, return NaN point
    if (dot == 0)
        return result;

    // find a = ((p2-p1)xV2).(V1xV2) / (V1xV2).(V1xV2)
    float alpha = glm::dot(v3, v4) / dot; //float alpha = v3.dot(v4) / dot;

    /*
    // if both V1 and V2 are same direction, return NaN point
    if(v4.x == 0 && v4.y == 0 && v4.z == 0)
        return result;

    float alpha = 0;
    if(v4.x != 0)
        alpha = v3.x / v4.x;
    else if(v4.y != 0)
        alpha = v3.y / v4.y;
    else if(v4.z != 0)
        alpha = v3.z / v4.z;
    else
        return result;
    */

    // find intersect point
    result = m_point + (alpha * m_direction);
    return result;
}

bool ivf::Line::isIntersected(const Line& line)
{
    // if 2 lines are same direction, the magnitude of cross product is 0
    //Vector3 v = this->direction.cross(line.getDirection());
    glm::vec3 v = glm::cross(m_direction, line.direction());
    if (v.x == 0 && v.y == 0 && v.z == 0)
        return false;
    else
        return true;
}

ivf::Plane::Plane()
    :m_normal(0.0f, 0.0f, 1.0f), 
     m_d(0.0f),
     m_normalLength(1.0f),
     m_distance(0.0f)
{
}

ivf::Plane::Plane(float a, float b, float c, float d)
{
    set(a, b, c, d);
}

ivf::Plane::Plane(const glm::vec3& normal, const glm::vec3& point)
{
    set(normal, point);
}

void ivf::Plane::set(float a, float b, float c, float d)
{
    m_normal.x = a;
    m_normal.y = b;
    m_normal.z = c;
    m_d = d;

    m_normalLength = sqrtf(a * a + b * b + c * c);
    m_distance = -m_d / m_normalLength;
}

void ivf::Plane::set(const glm::vec3& normal, const glm::vec3& point)
{
    m_normal = normal;
    m_normalLength = glm::length(m_normal); // normal.length();
    m_d = -glm::dot(m_normal, point);
    m_distance = -m_d / m_normalLength;
}

float ivf::Plane::distance(const glm::vec3& point)
{
    return (glm::dot(m_normal, point) + m_d) / m_normalLength;
}

void ivf::Plane::normalize()
{
    float lengthInv = 1.0f / m_normalLength;
    m_normal *= lengthInv;
    m_normalLength = 1.0f;
    m_d *= lengthInv;
    m_distance = -m_d;
}

glm::vec3 ivf::Plane::intersect(const Line& line) const
{
    // from line = p + t * v
    glm::vec3 p = line.point();        // (x0, y0, z0)
    glm::vec3 v = line.direction();    // (x,  y,  z)

    // dot products
    float dot1 = glm::dot(m_normal, p); // normal.dot(p);         // a*x0 + b*y0 + c*z0
    float dot2 = glm::dot(m_normal, v); // normal.dot(v);         // a*x + b*y + c*z

    // if denominator=0, no intersect
    if (dot2 == 0)
        return glm::vec3(NAN, NAN, NAN);

    // find t = -(a*x0 + b*y0 + c*z0 + d) / (a*x + b*y + c*z)
    float t = -(dot1 + m_d) / dot2;

    // find intersection point
    return p + (v * t);
}

ivf::Line ivf::Plane::intersect(const Plane& plane) const
{
    // find direction vector of the intersection line
    glm::vec3 v = glm::cross(m_normal, plane.normal()); // normal.cross(rhs.getNormal());

    // if |direction| = 0, 2 planes are parallel (no intersect)
    // return a line with NaN
    if (v.x == 0 && v.y == 0 && v.z == 0)
        return Line(glm::vec3(NAN, NAN, NAN), glm::vec3(NAN, NAN, NAN));

    // find a point on the line, which is also on both planes
    // choose simple plane where d=0: ax + by + cz = 0
    float dot = glm::dot(v, v); // v.dot(v);                       // V dot V
    glm::vec3 n1 = plane.d() * m_normal;           // d2 * N1
    glm::vec3 n2 = -m_d * plane.normal();          //-d1 * N2
    glm::vec3 p = glm::cross(n1 + n2, v) / dot;   //  (n1 + n2).cross(v) / dot;       // (d2*N1-d1*N2) X V / V dot V

    return Line(v, p);
}

bool ivf::Plane::isIntersected(const Line& line) const
{
    // direction vector of line
    glm::vec3 v = line.direction();

    // dot product with normal of the plane
    float dot = glm::dot(m_normal, v); // normal.dot(v);  // a*Vx + b*Vy + c*Vz

    if (dot == 0)
        return false;
    else
        return true;
}

bool ivf::Plane::isIntersected(const Plane& plane) const
{
    // check if 2 plane normals are same direction
    glm::vec3 cross = glm::cross(m_normal, plane.normal()); // normal.cross(plane.getNormal());

    if (cross.x == 0 && cross.y == 0 && cross.z == 0)
        return false;
    else
        return true;
}

const float EPSILON = 0.00001f;

glm::mat4 ivf::lookAt(const glm::vec3& target)
{
    // compute forward vector and normalize
    glm::vec3 position = glm::vec3(0.f, 0.0f, 0.0f);
    glm::vec3 forward = target - position;
    forward = glm::normalize(forward);
    glm::vec3 up;             // up vector of object
    glm::vec3 left;           // left vector of object

    // compute temporal up vector
    // if forward vector is near Y-axis, use up vector (0,0,-1) or (0,0,1)
    if (fabs(forward.x) < EPSILON && fabs(forward.z) < EPSILON)
    {
        // forward vector is pointing +Y axis
        if (forward.y > 0)
        {
            up.x = 0.0f;
            up.y = 0.0f;
            up.z = -1.0f;
        }
        // forward vector is pointing -Y axis
        else
        {
            up.x = 0.0f;
            up.y = 0.0f;
            up.z = 1.0f;
        }
            
    }
    else
    {
        // assume up vector is +Y axis
        up.x = 0.0f;
        up.y = 1.0f;
        up.z = 0.0f;
    }

    // compute left vector
    left = glm::normalize(glm::cross(up, forward));

    // re-compute up vector
    up = glm::cross(forward, left);
    //up.normalize();

    // NOTE: overwrite rotation and scale info of the current matrix

    glm::mat4 matrix(1.0f);

    matrix[0].x = left.x;
    matrix[0].y = left.y;
    matrix[0].z = left.z;
    matrix[1].x = up.x;
    matrix[1].y = up.y;
    matrix[1].z = up.z;
    matrix[2].x = forward.x;
    matrix[2].y = forward.y;
    matrix[2].z = forward.z;

    return matrix;
}
