#include "test_main.h"

#include <generator/generator.hpp>

IVF_TEST(sphere_vertices_on_surface)
{
    const double radius = 2.0;
    generator::SphereMesh sphere(radius, 16, 8);

    int n = 0;
    for (const auto &v : sphere.vertices())
    {
        IVF_CHECK_NEAR(gml::length(v.position), radius, 1e-9);
        IVF_CHECK_NEAR(gml::length(v.normal), 1.0, 1e-9);
        ++n;
    }
    IVF_CHECK(n > 0);
    IVF_CHECK(n == generator::count(sphere.vertices()));
}

IVF_TEST(sphere_triangle_indices_in_range)
{
    generator::SphereMesh sphere(1.0, 12, 6);
    const int nVerts = generator::count(sphere.vertices());

    int nTris = 0;
    for (const auto &t : sphere.triangles())
    {
        for (int i = 0; i < 3; ++i)
            IVF_CHECK(t.vertices[i] >= 0 && t.vertices[i] < nVerts);
        ++nTris;
    }
    IVF_CHECK(nTris > 0);
}

IVF_TEST(box_is_closed_and_bounded)
{
    generator::BoxMesh box(gml::dvec3(1.0, 2.0, 3.0));
    for (const auto &v : box.vertices())
    {
        IVF_CHECK(std::fabs(v.position[0]) <= 1.0 + 1e-9);
        IVF_CHECK(std::fabs(v.position[1]) <= 2.0 + 1e-9);
        IVF_CHECK(std::fabs(v.position[2]) <= 3.0 + 1e-9);
    }
    IVF_CHECK(generator::count(box.triangles()) >= 12);
}

IVF_TEST_MAIN()
