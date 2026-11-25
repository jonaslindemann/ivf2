// ivfvis without OpenGL: data sets, grids, vector fields, colormaps, ticks and vertex colouring.

#include "test_main.h"

#include <ivfvis/ivfvis.h>

#include <ivf/colors.h>
#include <ivf/picking.h>
#include <ivf/render_context.h>
#include <ivf/sphere.h>

#include <cmath>
#include <limits>
#include <stdexcept>

using namespace ivfvis;

namespace {

bool near(const glm::vec4 &a, const glm::vec4 &b, float eps = 1e-3f)
{
    return glm::length(a - b) <= eps;
}

bool near(const glm::vec3 &a, const glm::vec3 &b, float eps = 1e-3f)
{
    return glm::length(a - b) <= eps;
}

} // namespace

IVF_TEST(uniform_grid_layout)
{
    auto grid = UniformGrid::create({4, 3, 2}, {1.0f, 2.0f, 3.0f}, {0.5f, 1.0f, 2.0f});
    IVF_CHECK(grid->pointCount() == 24);
    IVF_CHECK(grid->index(1, 2, 1) == 1 + 4 * (2 + 3 * 1));
    IVF_CHECK(near(grid->position(1, 2, 1), {1.5f, 4.0f, 5.0f}));
    IVF_CHECK(near(grid->position(grid->index(3, 1, 1)), grid->position(3, 1, 1)));
    IVF_CHECK(near(grid->bounds().min(), {1.0f, 2.0f, 3.0f}));
    IVF_CHECK(near(grid->bounds().max(), {2.5f, 4.0f, 5.0f}));
    IVF_CHECK(grid->contains({2.0f, 3.0f, 4.0f}));
    IVF_CHECK(!grid->contains({0.0f, 3.0f, 4.0f}));

    auto boxed = UniformGrid::create({5, 5, 1}, ivf::BoundingBox({-1, -1, 0}, {1, 1, 0}));
    IVF_CHECK(near(boxed->spacing(), {0.5f, 0.5f, 1.0f}));
    IVF_CHECK(near(boxed->position(4, 4, 0), {1.0f, 1.0f, 0.0f}));
}

IVF_TEST(grid_sampling_is_trilinear)
{
    auto grid = UniformGrid::create({5, 6, 7}, {-1.0f, 0.0f, 2.0f}, {0.5f, 0.25f, 1.0f});
    // A linear function is reproduced exactly by trilinear interpolation
    auto f = [](const glm::vec3 &p) { return 2.0f * p.x - 3.0f * p.y + 0.5f * p.z + 1.0f; };
    grid->fillScalars("f", f);
    for (const glm::vec3 p : {glm::vec3(-0.8f, 0.3f, 2.5f), glm::vec3(0.9f, 1.2f, 7.9f), glm::vec3(0.0f, 0.0f, 2.0f)})
        IVF_CHECK_NEAR(grid->sample("f", p), f(p), 1e-4);
    // Clamped outside
    IVF_CHECK_NEAR(grid->sample("f", {-5.0f, 0.0f, 2.0f}), f({-1.0f, 0.0f, 2.0f}), 1e-4);
    // Gradient of a linear function
    IVF_CHECK(near(grid->gradient("f", {0.1f, 0.6f, 4.3f}), {2.0f, -3.0f, 0.5f}, 1e-3f));
    IVF_CHECK(near(grid->gradient("f", {-1.0f, 0.0f, 2.0f}), {2.0f, -3.0f, 0.5f}, 1e-3f)); // corner

    grid->fillVectors("v", [](const glm::vec3 &p) { return glm::vec3(p.y, -p.x, 1.0f); });
    IVF_CHECK(near(grid->sampleVector("v", {0.3f, 0.7f, 3.0f}), {0.7f, -0.3f, 1.0f}));

    // 2D grid (one point along z)
    auto flat = UniformGrid::create({3, 3, 1});
    flat->fillScalars("s", [](const glm::vec3 &p) { return p.x + p.y; });
    IVF_CHECK_NEAR(flat->sample("s", {0.5f, 1.5f, 9.0f}), 2.0f, 1e-5);
    IVF_CHECK(near(flat->gradient("s", {1.0f, 1.0f, 0.0f}), {1.0f, 1.0f, 0.0f}));
}

IVF_TEST(data_set_arrays)
{
    auto points = PointSet::create({{0, 0, 0}, {3, 4, 0}});
    auto s = points->addScalars("s", 1.0f);
    IVF_CHECK(s.size() == 2 && s[1] == 1.0f);
    points->fillVectors("v", [](const glm::vec3 &p) { return p; });
    points->addMagnitude("v", "speed");
    IVF_CHECK(points->scalars("speed")[1] == 5.0f);
    IVF_CHECK(points->range("v").min == 0.0f && points->range("v").max == 5.0f);
    IVF_CHECK(points->hasScalars("s") && !points->hasScalars("t") && points->hasVectors("v"));

    // New points extend every array
    points->addPoint({1, 1, 1});
    IVF_CHECK(points->scalars("s").size() == 3 && points->scalars("s")[2] == 0.0f);
    IVF_CHECK(points->bounds().max().y == 4.0f);

    // NaN is ignored by range(); missing arrays throw
    points->scalars("s")[0] = std::numeric_limits<float>::quiet_NaN();
    IVF_CHECK(points->range("s").min == 0.0f && points->range("s").max == 1.0f);
    bool threw = false;
    try
    {
        static_cast<void>(points->scalars("missing"));
    }
    catch (const std::out_of_range &)
    {
        threw = true;
    }
    IVF_CHECK(threw);

    points->removeArray("s");
    IVF_CHECK(!points->hasScalars("s"));
    IVF_CHECK(points->scalarNames() == std::vector<std::string>{"speed"});
}

IVF_TEST(vector_fields)
{
    auto grid = UniformGrid::create({3, 3, 3}, ivf::BoundingBox({-1, -1, -1}, {1, 1, 1}));
    grid->fillVectors("v", [](const glm::vec3 &p) { return glm::vec3(-p.y, p.x, 0.0f); });
    auto field = VectorField::fromGrid(grid, "v");
    IVF_CHECK(near(field->sample({0.5f, 0.25f, 0.0f}), {-0.25f, 0.5f, 0.0f}));
    IVF_CHECK(field->contains({0.0f, 0.0f, 0.0f}) && !field->contains({2.0f, 0.0f, 0.0f}));

    auto f = VectorField::fromFunction([](const glm::vec3 &p) { return 2.0f * p; });
    IVF_CHECK(near(f->sample({1, 2, 3}), {2, 4, 6}));
    IVF_CHECK(f->contains({1e6f, 0, 0})); // unbounded

    auto flow = VectorField::fromFlowField(ivf::FlowField::create(), ivf::BoundingBox({0, 0, 0}, {1, 1, 1}));
    const glm::vec3 v = flow->sample({0.3f, 0.4f, 0.5f});
    IVF_CHECK(std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z));
}

IVF_TEST(builtin_colormaps)
{
    IVF_CHECK(Colormap::builtinNames().size() == static_cast<std::size_t>(BuiltinColormapCount));
    auto viridis = Colormap::create(BuiltinColormap::Viridis);
    IVF_CHECK(viridis->name() == "viridis");
    // matplotlib's viridis: #440154 at 0, #fde725 at 1, #21918c in the middle
    IVF_CHECK(near(viridis->colorAt(0.0f), glm::vec4(0x44, 0x01, 0x54, 255) / 255.0f, 0.005f));
    IVF_CHECK(near(viridis->colorAt(1.0f), glm::vec4(0xfd, 0xe7, 0x25, 255) / 255.0f, 0.005f));
    IVF_CHECK(near(viridis->colorAt(0.5f), glm::vec4(0x21, 0x91, 0x8c, 255) / 255.0f, 0.01f));

    for (const auto &name : Colormap::builtinNames())
    {
        auto map = Colormap::named(name);
        IVF_CHECK(map.has_value() && (*map)->name() == name);
    }
    IVF_CHECK(Colormap::named("Grey").has_value());
    IVF_CHECK(!Colormap::named("no-such-map").has_value());

    auto gray = Colormap::create(BuiltinColormap::Gray);
    IVF_CHECK(near(gray->colorAt(0.25f), {0.25f, 0.25f, 0.25f, 1.0f}));
}

IVF_TEST(colormap_mapping)
{
    auto cmap = Colormap::create(BuiltinColormap::Gray);
    cmap->setRange(10.0f, 20.0f);
    IVF_CHECK_NEAR(cmap->normalize(15.0f), 0.5, 1e-6);
    IVF_CHECK_NEAR(cmap->value(0.25f), 12.5, 1e-5);
    IVF_CHECK(near(cmap->color(12.5f), {0.25f, 0.25f, 0.25f, 1.0f}));
    IVF_CHECK(near(cmap->color(0.0f), {0, 0, 0, 1}));    // clamped
    IVF_CHECK(near(cmap->color(100.0f), {1, 1, 1, 1})); // clamped

    const std::uint64_t v = cmap->version();
    cmap->setOutOfRangeColors(glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1));
    IVF_CHECK(cmap->version() != v);
    IVF_CHECK(near(cmap->color(0.0f), {1, 0, 0, 1}));
    IVF_CHECK(near(cmap->color(100.0f), {0, 0, 1, 1}));
    IVF_CHECK(near(cmap->color(std::numeric_limits<float>::quiet_NaN()), cmap->nanColor()));

    cmap->setReversed(true);
    IVF_CHECK(near(cmap->color(12.5f), {0.75f, 0.75f, 0.75f, 1.0f}));
    cmap->setReversed(false);

    // Four bands: 0, 1/3, 2/3, 1
    cmap->setSteps(4);
    IVF_CHECK(near(cmap->colorAt(0.1f), {0, 0, 0, 1}));
    IVF_CHECK(near(cmap->colorAt(0.3f), {1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 1.0f}));
    IVF_CHECK(near(cmap->colorAt(1.0f), {1, 1, 1, 1}));
    cmap->setSteps(0);

    // Logarithmic
    cmap->setRange(1.0f, 1000.0f);
    cmap->setScale(ColormapScale::Log);
    IVF_CHECK_NEAR(cmap->normalize(10.0f), 1.0 / 3.0, 1e-5);
    IVF_CHECK_NEAR(cmap->value(2.0f / 3.0f), 100.0, 1e-2);
    IVF_CHECK(cmap->normalize(-1.0f) < 0.0f);

    // Custom stops, unsorted, positions rescaled to 0..1
    auto custom = Colormap::create("custom", {{10.0f, {0, 0, 1, 1}}, {0.0f, {1, 0, 0, 1}}, {5.0f, {0, 1, 0, 1}}});
    IVF_CHECK(near(custom->colorAt(0.5f), {0, 1, 0, 1}));
    IVF_CHECK(near(custom->colorAt(0.25f), {0.5f, 0.5f, 0, 1}));
    auto even = Colormap::create("even", std::vector<glm::vec3>{{0, 0, 0}, {1, 1, 1}});
    IVF_CHECK(even->table(3).size() == 3 && near(even->table(3)[1], {0.5f, 0.5f, 0.5f, 1.0f}));
    const auto bytes = even->rgba8(2);
    IVF_CHECK(bytes.size() == 8 && bytes[0] == 0 && bytes[4] == 255 && bytes[7] == 255);

    auto copy = custom->copy();
    copy->setReversed(true);
    IVF_CHECK(!custom->reversed());
}

IVF_TEST(nice_ticks)
{
    IVF_CHECK(niceNumber(0.63, true) == 0.5);
    IVF_CHECK(niceNumber(0.73, true) == 1.0);
    IVF_CHECK(niceNumber(0.73, false) == 1.0);
    IVF_CHECK(niceNumber(23.0, true) == 20.0);

    auto ticks = niceTicks(0.0, 1.0, 5);
    IVF_CHECK(ticks.size() == 6 && ticks.front() == 0.0 && std::abs(ticks.back() - 1.0) < 1e-12);
    ticks = niceTicks(-3.7, 12.2, 5);
    IVF_CHECK(ticks.front() == 0.0 || ticks.front() < 0.0);
    IVF_CHECK(ticks.front() >= -3.7 && ticks.back() <= 12.2);
    for (std::size_t i = 1; i < ticks.size(); ++i)
        IVF_CHECK(std::abs((ticks[i] - ticks[i - 1]) - (ticks[1] - ticks[0])) < 1e-9);
    IVF_CHECK(niceTicks(2.0, 2.0).size() == 1);
    IVF_CHECK(niceTicks(3.0, 2.0).empty());

    IVF_CHECK((logTicks(1.0, 1000.0) == std::vector<double>{1.0, 10.0, 100.0, 1000.0}));
    const auto narrow = logTicks(1.5, 8.0);
    IVF_CHECK(narrow.size() >= 2 && narrow.front() >= 1.5 && narrow.back() <= 8.0);

    IVF_CHECK(formatTick(0.5, 0.1) == "0.5");
    IVF_CHECK(formatTick(2.0, 0.5) == "2.0");
    IVF_CHECK(formatTick(250.0, 50.0) == "250");
    IVF_CHECK(formatTick(1e-17, 0.2) == "0");
    IVF_CHECK(formatTick(0.00002, 0.00001) == "2e-05");
    IVF_CHECK(formatTick(3e7, 1e7) == "3e+07");
}

IVF_TEST(vertex_colors_from_colormap)
{
    auto ctx = ivf::RenderContext::create(nullptr);
    ctx->makeCurrent();

    auto sphere = ivf::Sphere::create(1.0, 16, 8);
    auto cmap = Colormap::create(BuiltinColormap::Gray);
    cmap->setRange(-1.0f, 1.0f);
    colorVertices(*sphere, [](const glm::vec3 &p) { return p.y; }, *cmap);
    useVertexColors(*sphere);
    IVF_CHECK(sphere->material() && sphere->material()->useVertexColor());

    auto mesh = sphere->meshes().at(0);
    auto vertices = mesh->vertices();
    float r = 0, g = 0, b = 0, a = 0;
    for (GLuint i = 0; i < mesh->vertexCount(); i += 7)
    {
        mesh->colors()->getColor(i, r, g, b, a);
        IVF_CHECK_NEAR(r, (vertices->vertex(i).y + 1.0f) * 0.5f, 1e-3);
    }

    // From grid data
    auto grid = UniformGrid::create({3, 3, 3}, ivf::BoundingBox({-1, -1, -1}, {1, 1, 1}));
    grid->fillScalars("x", [](const glm::vec3 &p) { return p.x; });
    colorVertices(*sphere, *grid, "x", *cmap);
    mesh->colors()->getColor(3, r, g, b, a);
    IVF_CHECK_NEAR(r, (vertices->vertex(3).x + 1.0f) * 0.5f, 1e-3);

    ivf::RenderContext::clearCurrent();
}

IVF_TEST(point_cloud_bounds_and_picking)
{
    auto ctx = ivf::RenderContext::create(nullptr);
    ctx->makeCurrent();

    auto points = PointSet::create({{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {1, 0, -5}});
    points->fillScalars("w", [](const glm::vec3 &p) { return p.x; });
    auto cloud = PointCloud::create(points);
    cloud->setPointSize(0.2f);
    IVF_CHECK(cloud->pointCount() == 4);

    // Bounds include the point radius
    IVF_CHECK(near(cloud->localBoundingBox().min(), {-0.1f, -0.1f, -5.1f}));
    IVF_CHECK(near(cloud->localBoundingBox().max(), {2.1f, 0.1f, 0.1f}));

    // A ray down the z axis through x = 1 hits the near point (z = 0) on its surface
    ivf::RayHit hit;
    const ivf::Ray ray{{1.0f, 0.0f, 10.0f}, {0.0f, 0.0f, -1.0f}};
    auto result = cloud->intersectRay(ray, glm::mat4(1.0f), 1e9f, hit);
    IVF_CHECK(result.has_value() && *result);
    IVF_CHECK(hit.point == 1);
    IVF_CHECK_NEAR(hit.distance, 9.9, 1e-4);
    IVF_CHECK(near(hit.normal, {0, 0, 1}));

    // Missing just outside the radius; maxDistance is respected
    const ivf::Ray beside{{1.15f, 0.0f, 10.0f}, {0.0f, 0.0f, -1.0f}};
    IVF_CHECK(!*cloud->intersectRay(beside, glm::mat4(1.0f), 1e9f, hit));
    IVF_CHECK(!*cloud->intersectRay(ray, glm::mat4(1.0f), 5.0f, hit));

    // Through pick(): the node's world transform is used
    cloud->setPos({0.0f, 3.0f, 0.0f});
    ivf::RenderQueue queue;
    cloud->collect(queue, ivf::CollectState{});
    const ivf::Ray moved{{2.0f, 3.0f, 10.0f}, {0.0f, 0.0f, -1.0f}};
    auto picked = ivf::pick(queue, moved);
    IVF_CHECK(picked && picked.node == cloud.get() && picked.point == 2);
    IVF_CHECK(!ivf::pick(queue, ray));

    // Size by an array: radius factor from 0.25 (x = 0) to 1 (x = 2)
    cloud->setSizeBy("w", 0.25f, 1.0f);
    const ivf::Ray edge{{0.08f, 3.0f, 10.0f}, {0.0f, 0.0f, -1.0f}}; // inside 0.1, outside 0.025
    IVF_CHECK(!ivf::pick(queue, edge));

    ivf::RenderContext::clearCurrent();
}

IVF_TEST_MAIN()
