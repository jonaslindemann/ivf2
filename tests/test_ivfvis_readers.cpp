// ivfvis readers: CSV, NumPy and VTK files (written by VTK and numpy, see data/make_test_data.py).

#include "test_main.h"

#include <ivfvis/readers.h>

#include <ivf/mesh.h>
#include <ivf/render_context.h>

#include <cmath>
#include <string>

using namespace ivfvis;

namespace {

std::filesystem::path data(const std::string &name)
{
    return std::filesystem::path(IVF2_TEST_DATA_DIR) / name;
}

bool near(const glm::vec3 &a, const glm::vec3 &b, float eps = 1e-4f)
{
    return glm::length(a - b) <= eps;
}

/** The test grid: 4 x 3 x 2 points, origin (1, 2, 3), spacing (0.5, 1, 2), f = x + 10y + 100z, v = position */
void checkGrid(const VtkData &d, const char *file)
{
    IVF_CHECK(d.grid != nullptr);
    if (!d.grid)
    {
        std::printf("  in %s\n", file);
        return;
    }
    IVF_CHECK(d.grid->dimensions() == glm::ivec3(4, 3, 2));
    IVF_CHECK(near(d.grid->origin(), {1, 2, 3}) && near(d.grid->spacing(), {0.5f, 1, 2}));
    const bool hasArrays = d.grid->hasScalars("f") && d.grid->hasVectors("v");
    IVF_CHECK(hasArrays);
    if (!hasArrays)
    {
        std::printf("  in %s\n", file);
        return;
    }
    int bad = 0;
    for (std::size_t i = 0; i < d.grid->pointCount(); ++i)
    {
        const glm::vec3 p = d.grid->position(i);
        bad += std::abs(d.grid->scalars("f")[i] - (p.x + 10 * p.y + 100 * p.z)) > 1e-3f ? 1 : 0;
        bad += near(d.grid->vectors("v")[i], p) ? 0 : 1;
    }
    IVF_CHECK(bad == 0);
    if (bad)
        std::printf("  %d wrong values in %s\n", bad, file);
}

/** The test sphere: 34 points, 64 triangles, height = z */
void checkSphere(const VtkData &d, const char *file)
{
    IVF_CHECK(d.points != nullptr && d.points->pointCount() == 34 && d.triangles.size() == 64);
    if (!d.points || !d.points->hasScalars("height"))
    {
        std::printf("  in %s\n", file);
        IVF_CHECK(false);
        return;
    }
    int bad = 0;
    for (std::size_t i = 0; i < 34; ++i)
        bad += std::abs(d.points->scalars("height")[i] - d.points->position(i).z) > 1e-5f ? 1 : 0;
    for (const glm::uvec3 &t : d.triangles)
        bad += (t.x < 34 && t.y < 34 && t.z < 34) ? 0 : 1;
    IVF_CHECK(bad == 0);
    if (bad)
        std::printf("  %d wrong values in %s\n", bad, file);
}

} // namespace

IVF_TEST(csv_tables)
{
    auto table = readCsv(data("points.csv"));
    IVF_CHECK(table.has_value());
    IVF_CHECK(table->rowCount() == 5);
    IVF_CHECK((table->names == std::vector<std::string>{"x", "y", "z", "temperature", "label"}));
    IVF_CHECK(table->values("temperature")[2] == 22.5f);
    IVF_CHECK(std::isnan(table->values("label")[0]));
    IVF_CHECK(table->column("missing") == -1 && table->values("missing").empty());

    auto points = pointsFromTable(*table, "x", "y", "z");
    IVF_CHECK(points.has_value() && (*points)->pointCount() == 5);
    IVF_CHECK(near((*points)->position(3), {3, 6, 9}));
    IVF_CHECK((*points)->hasScalars("temperature") && !(*points)->hasScalars("label") && !(*points)->hasScalars("x"));
    IVF_CHECK(!pointsFromTable(*table, "x", "nope", "z").has_value());

    auto semicolon = readCsv(data("table_semicolon.csv"));
    IVF_CHECK(semicolon.has_value() && semicolon->rowCount() == 3 && semicolon->names.size() == 3);
    IVF_CHECK(semicolon->names[1] == "column2" && semicolon->values("column3")[2] == 9.0f);

    auto missing = readCsv(data("nothing.csv"));
    IVF_CHECK(!missing.has_value() && missing.error().code == ivf::ErrorCode::FileNotFound);
}

IVF_TEST(numpy_arrays)
{
    auto f32 = readNpy(data("field_f32.npy"));
    IVF_CHECK(f32.has_value() && (f32->shape == std::vector<std::size_t>{2, 3, 4}) && f32->dtype == "<f4");
    auto grid = gridFromNpy(*f32, "f");
    IVF_CHECK(grid.has_value() && (*grid)->dimensions() == glm::ivec3(4, 3, 2));
    IVF_CHECK((*grid)->scalars("f")[(*grid)->index(3, 2, 1)] == 3.0f + 4 * 2 + 12 * 1);

    auto fortran = readNpy(data("field_f64_fortran.npy"));
    IVF_CHECK(fortran.has_value() && fortran->fortranOrder);
    auto fgrid = gridFromNpy(*fortran, "f", {1, 1, 1}, {0.5f, 0.5f, 0.5f});
    IVF_CHECK(fgrid.has_value() && (*fgrid)->dimensions() == glm::ivec3(4, 3, 2));
    IVF_CHECK((*fgrid)->scalars("f")[(*fgrid)->index(1, 2, 1)] == 1.0f + 4 * 2 + 12 * 1);
    IVF_CHECK(near((*fgrid)->origin(), {1, 1, 1}));

    auto ints = readNpy(data("points_i32.npy"));
    auto points = pointsFromNpy(*ints);
    IVF_CHECK(points.has_value() && (*points)->pointCount() == 5 && near((*points)->position(2), {6, 7, 8}));

    auto vectors = gridFromNpy(*readNpy(data("vectors_f32.npy")), "v");
    IVF_CHECK(vectors.has_value() && (*vectors)->dimensions() == glm::ivec3(4, 3, 2));
    IVF_CHECK(near((*vectors)->vectors("v")[5], {1, 1, 1}));

    auto big = readNpy(data("bigendian_f8.npy"));
    IVF_CHECK(big.has_value() && big->data.size() == 6 && big->data[5] == 5.0f);

    IVF_CHECK(!readNpy(data("points.csv")).has_value()); // not a .npy file
    IVF_CHECK(!pointsFromNpy(*f32).has_value());         // wrong shape
}

IVF_TEST(vtk_legacy)
{
    for (const char *file : {"grid_42_ascii.vtk", "grid_42_binary.vtk", "grid_51_binary.vtk"})
    {
        auto d = readVtk(data(file));
        IVF_CHECK(d.has_value());
        if (d)
            checkGrid(*d, file);
    }
    for (const char *file : {"sphere_42_ascii.vtk", "sphere_51_ascii.vtk", "sphere_51_binary.vtk", "sphere_ugrid_42_ascii.vtk"})
    {
        auto d = readVtk(data(file));
        IVF_CHECK(d.has_value());
        if (d)
            checkSphere(*d, file);
        else
            std::printf("  %s: %s\n", file, d.error().message.c_str());
    }
    auto missing = readVtk(data("nothing.vtk"));
    IVF_CHECK(!missing.has_value() && missing.error().code == ivf::ErrorCode::FileNotFound);
    IVF_CHECK(!readVtk(data("points.csv")).has_value());
}

IVF_TEST(vtk_xml)
{
    const char *encodings[] = {"ascii", "binary", "binary_zlib", "appended_raw", "appended_zlib", "appended_base64"};
#ifdef IVF2_TEST_VTK_XML
    for (const char *encoding : encodings)
    {
        const std::string grid = std::string("grid_") + encoding + ".vti";
        auto g = readVtk(data(grid));
        IVF_CHECK(g.has_value());
        if (g)
            checkGrid(*g, grid.c_str());
        else
            std::printf("  %s: %s\n", grid.c_str(), g.error().message.c_str());

        const std::string sphere = std::string("sphere_") + encoding + ".vtu";
        auto s = readVtk(data(sphere));
        IVF_CHECK(s.has_value());
        if (s)
            checkSphere(*s, sphere.c_str());
        else
            std::printf("  %s: %s\n", sphere.c_str(), s.error().message.c_str());
    }
    auto quads = readVtk(data("quads_ascii.vtu"));
    IVF_CHECK(quads.has_value() && quads->points->pointCount() == 9 && quads->triangles.size() == 8);
    IVF_CHECK(quads->points->scalars("s")[7] == 7.0f);
#else
    // Built without VTK XML support: a clear error
    for (const char *encoding : encodings)
    {
        auto g = readVtk(data(std::string("grid_") + encoding + ".vti"));
        IVF_CHECK(!g.has_value() && g.error().code == ivf::ErrorCode::NotSupported);
    }
#endif
}

IVF_TEST(mesh_from_triangles)
{
    auto ctx = ivf::RenderContext::create(nullptr);
    ctx->makeCurrent();
    auto d = readVtk(data("sphere_42_ascii.vtk"));
    IVF_CHECK(d.has_value());
    auto node = createMeshNode(*d->points, d->triangles);
    IVF_CHECK(node->meshes().size() == 1 && node->meshes()[0]->vertexCount() == 34);
    IVF_CHECK(node->localBoundingBox().isValid());
    ivf::RenderContext::clearCurrent();
}

IVF_TEST_MAIN()
