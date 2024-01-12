// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include "generator/PolyLinePath.hpp"

using namespace generator;

gml::dvec3 interpolate(const std::vector<gml::dvec3> &polyline, double t)
{
    if (polyline.empty()) {
        // Handle empty polyline
        return gml::dvec3(0.0f, 0.0f, 0.0f);
    }

    if (t <= 0.0)
        return polyline.front();
    if (t >= 1.0)
        return polyline.back();

    double totalLength = 0.0;
    std::vector<double> lengths(polyline.size());
    for (size_t i = 1; i < polyline.size(); ++i) {
        double segmentLength = gml::length(polyline[i] - polyline[i - 1]);
        totalLength += segmentLength;
        lengths[i] = totalLength;
    }

    double targetLength = t * totalLength;
    for (size_t i = 1; i < polyline.size(); ++i) {
        if (lengths[i] >= targetLength) {
            double segmentLength = lengths[i] - lengths[i - 1];
            double segmentT = (targetLength - lengths[i - 1]) / segmentLength;
            return polyline[i - 1] + segmentT * (polyline[i] - polyline[i - 1]);
        }
    }

    return polyline.back();
}

PolyLinePath::PolyLinePath(int segments)
    : parametricPath_{[this, segments](double t) {
                          PathVertex vertex;

                          vertex.position = interpolate(points_, t);
                          gml::dvec3 start = interpolate(points_, t - 0.001);
                          gml::dvec3 end = interpolate(points_, t + 0.001);
                          vertex.tangent = normalize(end - start);
                          // vertex.normal = vertex.tangent;
                          vertex.normal = gml::cross(gml::dvec3(1.0, 1.0, 0.0), vertex.tangent);
                          vertex.texCoord = t;

                          return vertex;
                      },
                      segments},
      segments_{segments}
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
