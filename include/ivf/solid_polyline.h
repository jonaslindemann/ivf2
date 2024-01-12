#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

#include <generator/PolyLinePath.hpp>

namespace ivf {

class SolidPolyLine : public MeshNode {
private:
    double m_radius;
    generator::PolyLinePath m_polyPath;
    int m_segments;

public:
    SolidPolyLine(double radius = 0.1, int segments = 8);

    static std::shared_ptr<SolidPolyLine> create(double radius = 0.1, int segments = 8);

    void setRadius(double radius);

    void addPoint(const gml::dvec3 &point);
    void addPoint(double x, double y, double z);
    void setPoints(const std::vector<gml::dvec3> &points);
    void clearPoints();
    void setSegments(int segments);
    void setPoint(int index, const gml::dvec3 &point);
    gml::dvec3 point(int index) const;
    int pointCount();

protected:
    virtual void doSetup();
};

typedef std::shared_ptr<SolidPolyLine> SolidPolyLinePtr;

} // namespace ivf
