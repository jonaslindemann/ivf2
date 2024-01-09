// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifndef GENERATOR_POLYLINEPATH_HPP
#define GENERATOR_POLYLINEPATH_HPP

#include "ParametricPath.hpp"

#include <vector>

namespace generator {

/// A path from point to point.
/// @image html LinePath.svg
class PolyLinePath {
private:
    using Impl = ParametricPath;
    Impl parametricPath_;

    std::vector<gml::dvec3> points_;

public:
    /// @param start Start point of the line.
    /// @param end End point of the line.
    /// @param normal Line normal. Should be parallel to the line.
    /// @param segments Number of subdivisions along the line.
    PolyLinePath(int segments = 8);

    void addPoint(const gml::dvec3 &point);
    void setPoints(const std::vector<gml::dvec3> &points);
    void clearPoints();
    void setSegments(int segments);
    void setPoint(int index, const gml::dvec3 &point);
    gml::dvec3 point(int index) const;
    int pointCount();

    using Edges = typename Impl::Edges;

    Edges edges() const noexcept
    {
        return parametricPath_.edges();
    }

    using Vertices = typename Impl::Vertices;

    Vertices vertices() const noexcept
    {
        return parametricPath_.vertices();
    }
};

} // namespace generator

#endif
