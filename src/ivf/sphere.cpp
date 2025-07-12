#include <ivf/sphere.h>

#include <generator/SphereMesh.hpp>

using namespace ivf;
using namespace generator;

Sphere::Sphere(double radius, int slices, int segments, double sliceStart, double sliceSweep, double segmentStart,
               double segmentSweep)
    : m_radius(radius), m_slices(slices), m_segments(segments), m_sliceStart(sliceStart), m_sliceSweep(sliceSweep),
      m_segmentStart(segmentStart), m_segmentSweep(segmentSweep)
{
    this->doSetup();
    this->setName("Sphere");
}

std::shared_ptr<Sphere> Sphere::create(double radius, int slices, int segments, double sliceStart, double sliceSweep,
                                       double segmentStart, double segmentSweep)
{
    return std::make_shared<Sphere>(radius, slices, segments, sliceStart, sliceSweep, segmentStart, segmentSweep);
}

void Sphere::doSetup()
{
    SphereMesh sphere(m_radius, m_slices, m_segments, m_sliceStart, m_sliceSweep, m_segmentStart, m_segmentSweep);

    AnyGenerator<MeshVertex> vertices = sphere.vertices();
    AnyGenerator<Triangle> triangles = sphere.triangles();

    this->createFromGenerator(vertices, triangles);
}

void Sphere::setRadius(double radius)
{
    m_radius = radius;
}

void Sphere::setSlices(int slices)
{
    m_slices = slices;
}

void Sphere::setSegments(int segments)
{
    m_segments = segments;
}

void ivf::Sphere::set(double radius, int slices, int segments, double sliceStart, double sliceSweep,
                      double segmentStart, double segmentSweep)
{
    m_radius = radius;
    m_slices = slices;
    m_segments = segments;
    m_sliceStart = sliceStart;
    m_sliceSweep = sliceSweep;
    m_segmentStart = segmentStart;
    m_segmentSweep = segmentSweep;
    this->refresh();
}

void Sphere::setSliceStart(double sliceStart)
{
    m_sliceStart = sliceStart;
}

void Sphere::setSliceSweep(double sliceSweep)
{
    m_sliceSweep = sliceSweep;
}

void Sphere::setSegmentStart(double segmentStart)
{
    m_segmentStart = segmentStart;
}

void Sphere::setSegmentSweep(double segmentSweep)
{
    m_segmentSweep = segmentSweep;
}
