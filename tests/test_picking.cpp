// CPU picking without OpenGL: rays, the mesh BVH (against brute force), and pick() on render queues.

#include "test_main.h"

#include <ivf/box.h>
#include <ivf/composite_node.h>
#include <ivf/cube.h>
#include <ivf/instance_node.h>
#include <ivf/mesh_bvh.h>
#include <ivf/picking.h>
#include <ivf/render_context.h>
#include <ivf/sphere.h>

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <limits>
#include <optional>
#include <random>
#include <vector>

using namespace ivf;

namespace {

bool near(const glm::vec3 &a, const glm::vec3 &b, float eps = 1e-3f)
{
    return glm::length(a - b) <= eps;
}

std::shared_ptr<RenderContext> makeContext()
{
    auto ctx = RenderContext::create(nullptr);
    ctx->makeCurrent();
    return ctx;
}

RenderQueue collect(const NodePtr &node)
{
    RenderQueue queue;
    node->collect(queue, CollectState{});
    return queue;
}

} // namespace

IVF_TEST(camera_rays)
{
    const glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0), glm::vec3(0, 1, 0));
    const glm::mat4 projection = glm::perspective(glm::radians(60.0f), 2.0f, 0.1f, 100.0f);
    const Ray center = rayFromScreen(400, 200, 800, 400, view, projection);
    IVF_CHECK(near(center.direction, {0, 0, -1}));
    IVF_CHECK(std::abs(center.origin.z - 9.9f) < 1e-3f);

    // The top left corner: up and to the left, at the corner of the view
    const Ray corner = rayFromScreen(0, 0, 800, 400, view, projection);
    IVF_CHECK(corner.direction.x < 0.0f && corner.direction.y > 0.0f);
    const float tanHalf = std::tan(glm::radians(30.0f));
    IVF_CHECK(std::abs(corner.direction.y / -corner.direction.z - tanHalf) < 1e-3f);
    IVF_CHECK(std::abs(corner.direction.x / corner.direction.z - 2.0f * tanHalf) < 1e-3f);
}

IVF_TEST(box_and_triangle_tests)
{
    float t0 = 0, t1 = 0;
    const Ray ray{{0, 0, 5}, {0, 0, -1}};
    IVF_CHECK(intersectBox(ray, glm::vec3(-1), glm::vec3(1), t0, t1));
    IVF_CHECK(std::abs(t0 - 4.0f) < 1e-6f && std::abs(t1 - 6.0f) < 1e-6f);
    IVF_CHECK(!intersectBox({{3, 0, 5}, {0, 0, -1}}, glm::vec3(-1), glm::vec3(1), t0, t1)); // beside
    IVF_CHECK(!intersectBox({{0, 0, 5}, {0, 0, 1}}, glm::vec3(-1), glm::vec3(1), t0, t1));  // behind
    IVF_CHECK(intersectBox({{0, 0, 0}, {0, 0, 1}}, glm::vec3(-1), glm::vec3(1), t0, t1));   // inside
    IVF_CHECK(!intersectBox(ray, BoundingBox(), t0, t1));                                    // invalid box

    float t = 0, u = 0, v = 0;
    const glm::vec3 a(-1, -1, 0), b(1, -1, 0), c(0, 1, 0);
    IVF_CHECK(intersectTriangle(ray, a, b, c, t, u, v));
    IVF_CHECK(std::abs(t - 5.0f) < 1e-6f);
    IVF_CHECK(near(a + u * (b - a) + v * (c - a), {0, 0, 0}));
    IVF_CHECK(intersectTriangle(ray, a, c, b, t, u, v)); // both sides
    IVF_CHECK(!intersectTriangle({{2, 2, 5}, {0, 0, -1}}, a, b, c, t, u, v));
    IVF_CHECK(!intersectTriangle({{0, 0, 5}, {1, 0, 0}}, a, b, c, t, u, v)); // parallel
}

IVF_TEST(bvh_matches_brute_force)
{
    std::mt19937 random(11);
    std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
    std::vector<glm::vec3> positions;
    std::vector<glm::uvec3> triangles;
    for (std::uint32_t i = 0; i < 3000; ++i)
    {
        const glm::vec3 center(unit(random) * 10.0f, unit(random) * 10.0f, unit(random) * 10.0f);
        for (int k = 0; k < 3; ++k)
            positions.push_back(center + glm::vec3(unit(random), unit(random), unit(random)) * 0.6f);
        triangles.emplace_back(3 * i, 3 * i + 1, 3 * i + 2);
    }
    const MeshBvh bvh(positions, triangles);
    IVF_CHECK(bvh.triangleCount() == 3000);
    IVF_CHECK(bvh.depth() > 5 && bvh.depth() < 40);

    int hits = 0, mismatches = 0;
    for (int r = 0; r < 2000; ++r)
    {
        const glm::vec3 origin(unit(random) * 15.0f, unit(random) * 15.0f, unit(random) * 15.0f);
        const Ray ray{origin, glm::normalize(-origin + glm::vec3(unit(random), unit(random), unit(random)) * 6.0f)};
        float best = std::numeric_limits<float>::infinity();
        for (const auto &tri : triangles)
        {
            float t, u, v;
            if (intersectTriangle(ray, positions[tri.x], positions[tri.y], positions[tri.z], t, u, v) && t < best)
                best = t;
        }
        const auto hit = bvh.intersect(ray);
        const bool bruteHit = std::isfinite(best);
        if (bruteHit != hit.has_value() || (hit && std::abs(hit->t - best) > 1e-4f))
            ++mismatches;
        if (bvh.occluded(ray) != bruteHit)
            ++mismatches;
        hits += bruteHit ? 1 : 0;
    }
    IVF_CHECK(mismatches == 0);
    IVF_CHECK(hits > 200); // the test rays do hit things

    // A limit on t
    const Ray down{{0, 0, 100}, {0, 0, -1}};
    if (auto first = bvh.intersect(down))
        IVF_CHECK(!bvh.intersect(down, first->t * 0.5f));

    IVF_CHECK(!MeshBvh().intersect(down));
}

IVF_TEST(mesh_bvh_follows_vertex_changes)
{
    auto ctx = makeContext();
    auto sphere = Sphere::create(1.0, 24, 12);
    auto mesh = sphere->meshes().at(0);
    auto bvh = mesh->bvh();
    IVF_CHECK(bvh != nullptr);
    IVF_CHECK(mesh->bvh() == bvh); // cached
    const Ray ray{{0, 0, 5}, {0, 0, -1}};
    IVF_CHECK(bvh->intersect(ray) && std::abs(bvh->intersect(ray)->t - 4.0f) < 0.02f);

    // Move every vertex: the next bvh() is rebuilt
    auto vertices = mesh->vertices();
    for (GLuint i = 0; i < vertices->rows(); ++i)
        vertices->setVertex(i, vertices->vertex(i) + glm::vec3(0, 0, 2));
    mesh->updateVertices();
    auto moved = mesh->bvh();
    IVF_CHECK(moved != bvh);
    IVF_CHECK(moved->intersect(ray) && std::abs(moved->intersect(ray)->t - 2.0f) < 0.02f);
    RenderContext::clearCurrent();
}

IVF_TEST(pick_finds_the_nearest_node)
{
    auto ctx = makeContext();
    auto scene = CompositeNode::create();
    auto front = Box::create({0.5f, 0.5f, 0.5f});
    front->setPos({0, 0, 2});
    front->setName("front");
    auto back = Cube::create();
    back->setPos({0, 0, -2});
    back->setName("back");
    auto turned = Box::create({0.5f, 0.5f, 0.5f});
    turned->setPos({3, 0, 0});
    turned->setEulerAngles(0.0f, 45.0f, 0.0f);
    turned->setScale({2, 2, 2});
    scene->add(front);
    scene->add(back);
    scene->add(turned);
    const RenderQueue queue = collect(scene);

    const Ray ray{{0, 0, 10}, {0, 0, -1}};
    const RayHit hit = pick(queue, ray);
    IVF_CHECK(hit && hit.node == front.get());
    IVF_CHECK(hit.exact && hit.mesh != nullptr && hit.triangle >= 0);
    IVF_CHECK(std::abs(hit.distance - 7.5f) < 1e-3f);
    IVF_CHECK(near(hit.position, {0, 0, 2.5f}));
    IVF_CHECK(near(hit.normal, {0, 0, 1}));

    // All hits along the ray, nearest first
    const auto all = pickAll(queue, ray);
    IVF_CHECK(all.size() == 2);
    IVF_CHECK(all.size() == 2 && all[1].node == back.get() && all[0].distance < all[1].distance);

    // Skipping the front box finds the one behind it; maxDistance limits the search
    PickOptions options;
    options.skip = [&](const DrawItem &item) { return item.node == front.get(); };
    IVF_CHECK(pick(queue, ray, options).node == back.get());
    PickOptions shortRange;
    shortRange.maxDistance = 5.0f;
    IVF_CHECK(!pick(queue, ray, shortRange));

    // The rotated, scaled box: hit on its corner edge facing +Z (half diagonal 0.5 * 2 * sqrt(2))
    const RayHit side = pick(queue, {{3, 0, 10}, {0, 0, -1}});
    IVF_CHECK(side.node == turned.get());
    IVF_CHECK(std::abs(side.position.z - std::sqrt(2.0f)) < 1e-3f);

    IVF_CHECK(!pick(queue, {{0, 5, 10}, {0, 0, -1}}));
    RenderContext::clearCurrent();
}

IVF_TEST(pick_instances_layers_and_bounds)
{
    auto ctx = makeContext();
    auto scene = CompositeNode::create();
    auto shared = Cube::create();
    for (int i = 0; i < 3; ++i)
    {
        auto copy = InstanceNode::create();
        copy->setNode(shared);
        copy->setPos({float(i) * 3.0f, 0, 0});
        scene->add(copy);
    }
    const RenderQueue queue = collect(scene);
    // Each copy is hit where it is drawn
    for (int i = 0; i < 3; ++i)
    {
        const RayHit hit = pick(queue, {{float(i) * 3.0f, 0, 10}, {0, 0, -1}});
        IVF_CHECK(hit && std::abs(hit.position.x - float(i) * 3.0f) < 1e-3f && std::abs(hit.position.z - 0.5f) < 1e-3f);
    }
    IVF_CHECK(!pick(queue, {{1.5f, 0, 10}, {0, 0, -1}}));

    // A background item (a sky around the camera) is never picked
    auto sky = Sphere::create(50.0);
    sky->setRenderLayer(RenderLayer::Background);
    auto withSky = CompositeNode::create();
    withSky->add(sky);
    IVF_CHECK(!pick(collect(withSky), {{0, 0, 0}, {0, 0, -1}}));

    // An item with bounds but no mesh: hit at its box (not exact)
    RenderQueue boxes;
    DrawItem item;
    item.node = scene.get();
    item.bounds = BoundingBox({-1, -1, -1}, {1, 1, 1});
    boxes.add(item);
    const RayHit boxHit = pick(boxes, {{0, 0, 10}, {0, 0, -1}});
    IVF_CHECK(boxHit && !boxHit.exact && boxHit.triangle == -1);
    IVF_CHECK(near(boxHit.position, {0, 0, 1}) && near(boxHit.normal, {0, 0, 1}));
    PickOptions exactOnly;
    exactOnly.boundsFallback = false;
    IVF_CHECK(!pick(boxes, {{0, 0, 10}, {0, 0, -1}}, exactOnly));
    RenderContext::clearCurrent();
}

IVF_TEST_MAIN()
