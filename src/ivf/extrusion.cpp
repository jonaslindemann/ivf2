#include <ivf/extrusion.h>

#include <ivf/light_manager.h>
#include <ivf/geometry.h>

#include <glm/gtx/intersect.hpp>

#include <iostream>

using namespace ivf;
using namespace std;

void ivf::Extrusion::generateContours()
{
    m_contours.clear();
    m_normals.clear();

    if (m_path.size() < 1)
        return;

    transformFirstContour();
    m_contours.push_back(this->m_contour);
    m_normals.push_back(computeContourNormal(0));

    for (auto i = 1; i < m_path.size(); ++i)
    {
        m_contours.push_back(projectContour(i - 1, i));
        m_normals.push_back(computeContourNormal(i));
    }
}

void ivf::Extrusion::transformFirstContour()
{
    int pathCount = (int)m_path.size();
    int vertexCount = (int)m_contour.size();

    glm::mat4 matrix(1.0f);
    //Matrix4 matrix;

    if (pathCount > 0)
    {
        // transform matrix
        if (pathCount > 1)
            matrix = lookAt(m_path[1] - m_path[0]);

        glm::mat4 translate = glm::translate(m_path[0]);

        matrix = matrix * translate;
        //matrix.translate(path[0]);

        // multiply matrix to the contour
        // NOTE: the contour vertices are transformed here
        //       MUST resubmit contour data if the path is resset to 0

        glm::vec4 v;

        for (int i = 0; i < vertexCount; ++i)
        {
            v = matrix * glm::vec4(m_contour[i], 1.0f);
            m_contour[i].x = v.x;
            m_contour[i].y = v.y;
            m_contour[i].z = v.z;
        }
    }
}

std::vector<glm::vec3> ivf::Extrusion::projectContour(int fromIndex, int toIndex)
{
    glm::vec3 dir1, dir2, normal;
    Line line;

    dir1 = m_path[toIndex] - m_path[fromIndex];

    if (toIndex == int(m_path.size()) - 1)
        dir2 = dir1;
    else
        dir2 = m_path[toIndex + 1] - m_path[toIndex];

    normal = dir1 + dir2; // normal vector of plane at toIndex

    Plane plane(normal, m_path[toIndex]);

    // project each vertex of contour to the plane

    std::vector<glm::vec3>& fromContour = m_contours[fromIndex];
    std::vector<glm::vec3> toContour;

    for (auto i = 0; i<int(fromContour.size()); ++i)
    {
        line.set(dir1, fromContour[i]);
        toContour.push_back(plane.intersect(line));
    }

    return toContour;
}

std::vector<glm::vec3> ivf::Extrusion::computeContourNormal(int pathIndex)
{
    // get current contour and center point
    std::vector<glm::vec3>& contour = m_contours[pathIndex];
    glm::vec3 center = m_path[pathIndex];

    std::vector<glm::vec3> contourNormal;
    glm::vec3 normal;
    for (int i = 0; i < (int)m_contour.size(); ++i)
    {
        normal = glm::normalize(contour[i] - center);
        contourNormal.push_back(normal);
    }

    return contourNormal;
}

glm::vec3 ivf::Extrusion::intersectPlane(glm::vec3& pos, glm::vec3& dir, glm::vec3& planePos, glm::vec3& planeDir)
{
    float d;
    bool intersects = glm::intersectRayPlane(pos, dir, planePos, planeDir, d);

    if (intersects)
    {
        glm::vec3 planePoint = pos + d * dir;
        return planePoint;
    }
    else
    {
        return glm::vec3(0.0, 0.0, 0.0);
    }
}

ivf::Extrusion::Extrusion()
{
    this->setUseTexture(false);
    this->setUseMaterial(true);
}

std::shared_ptr<Extrusion> ivf::Extrusion::create()
{
	return std::make_shared<Extrusion>();
}

void ivf::Extrusion::addPathPoint(glm::vec3 p)
{
	m_path.push_back(p);
}

void ivf::Extrusion::clearPath()
{
	m_path.clear();
}

void ivf::Extrusion::createCircleSection(float r, int segments)
{
    this->clearSection();

    float da = glm::pi<float>() / segments;
    float angle = 0.0f;

    float x, y;

    while (angle < glm::pi<float>() * 2.0)
    {
        x = r * cos(angle);
        y = r * sin(angle);
        this->addSectionPoint(glm::vec2(x, y));
        angle += da;
    }
}

void ivf::Extrusion::addSectionPoint(glm::vec2 p)
{
	glm::vec3 pp(p.x, p.y, 0.0);
	m_contour.push_back(pp);
}

void ivf::Extrusion::clearSection()
{
    m_contour.clear();
}

void ivf::Extrusion::doSetup()
{
    generateContours();

    /*
    this->newMesh(m_path.size() * m_contours[0].size() * 2);

    mesh()->begin(GL_LINES);
    for (auto i = 0; i < m_path.size(); i++)
    {
        std::vector<glm::vec3> contour = m_contours[i];
        std::vector<glm::vec3> normal = m_normals[i];

        for (auto j = 0; j < contour.size()-1; j++)
        {
            mesh()->color3f(1.0f, 1.0, 1.0f);
            mesh()->vertex3f(contour[j]);
            mesh()->normal3f(normal[j]);
            mesh()->color3f(1.0f, 1.0, 1.0f);
            mesh()->vertex3f(contour[j+1]);
            mesh()->normal3f(normal[j+1]);
        }
    }
    mesh()->end();
    */

    for (auto i = 0; i < m_path.size()-1; i++)
    {
        std::vector<glm::vec3> c1 = m_contours[i];
        std::vector<glm::vec3> c2 = m_contours[i+1];
        std::vector<glm::vec3> n1 = m_normals[i];
        std::vector<glm::vec3> n2 = m_normals[i+1];

        this->newMesh((c1.size()+1)*2);

        currentMesh()->begin(GL_TRIANGLE_STRIP);
        for (auto j = 0; j < c1.size(); j++)
        {
            currentMesh()->vertex3f(c1[j]);
            currentMesh()->normal3f(n1[j]);
            currentMesh()->vertex3f(c2[j]);
            currentMesh()->normal3f(n2[j]);
        }
        currentMesh()->vertex3f(c1[0]);
        currentMesh()->normal3f(n1[0]);
        currentMesh()->vertex3f(c2[0]);
        currentMesh()->normal3f(n2[0]);
        currentMesh()->end();
    }
}

void ivf::Extrusion::doPreDraw()
{
    MeshNode::doPreDraw();
}

void ivf::Extrusion::doPostDraw()
{
    MeshNode::doPostDraw();
}
