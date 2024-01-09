// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include "generator/PolyLinePath.hpp"

using namespace generator;

PolyLinePath::PolyLinePath(int segments)
    : parametricPath_{[this, segments](double t) {
                          PathVertex vertex;

                          // o ----- o ----- o ----- o ----- o
                          // 0       1       2       3       4

                          // size() == 5
                          // segments == 4
                          // 0.0 <= t < 1.0

                          double dt = 1.0 / (segments - 1);     // 0.25
                          int index = static_cast<int>(t / dt); // 0, 1, 2, 3
                          double t0 = index * dt;               // 0.0, 0.25, 0.5, 0.75
                          double t1 = (index + 1) * dt;         // 0.25, 0.5, 0.75, 1.0

                          gml::dvec3 start = this->points_[index];
                          gml::dvec3 end = this->points_[index + 1];

                          vertex.position = start + t * (end - start);
                          vertex.tangent = gml::normalize(end - start);
                          vertex.normal = gml::dvec3(0.0, 0.0, 1.0);
                          vertex.texCoord = t;

                          return vertex;
                      },
                      segments}
{
}

void generator::PolyLinePath::addPoint(const gml::dvec3 &point)
{
    points_.push_back(point);
}

void generator::PolyLinePath::setPoints(const std::vector<gml::dvec3> &points)
{
    points_ = points;
}

void generator::PolyLinePath::clearPoints()
{
    points_.clear();
}

void generator::PolyLinePath::setSegments(int segments)
{
    //
}

void generator::PolyLinePath::setPoint(int index, const gml::dvec3 &point)
{
    points_[index] = point;
}

gml::dvec3 generator::PolyLinePath::point(int index) const
{
    return points_[index];
}

int generator::PolyLinePath::pointCount()
{
    return 0;
}
