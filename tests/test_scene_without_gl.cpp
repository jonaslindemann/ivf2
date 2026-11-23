// Building geometry needs no OpenGL context: meshes are uploaded lazily when they are first
// drawn (roadmap phase 2, render queue step 1). No GL functions are loaded in this test, so
// any GL call while creating or changing nodes would crash it.

#include "test_main.h"

#include <ivf/box.h>
#include <ivf/composite_node.h>
#include <ivf/cube.h>
#include <ivf/mesh_node.h>
#include <ivf/render_context.h>
#include <ivf/sphere.h>

using namespace ivf;

namespace {

// A current RenderContext without loaded GL functions (avoids the fallback warning)
std::shared_ptr<RenderContext> makeContext()
{
    auto ctx = RenderContext::create(nullptr);
    ctx->makeCurrent();
    return ctx;
}

} // namespace

IVF_TEST(primitives_are_built_without_gl)
{
    auto ctx = makeContext();

    auto cube = Cube::create();
    auto sphere = Sphere::create(0.5);
    auto box = Box::create(glm::vec3(1.0f, 2.0f, 3.0f));

    IVF_CHECK(cube->mesh() != nullptr);
    IVF_CHECK(cube->mesh()->vertices()->rows() > 0);
    IVF_CHECK(sphere->mesh() != nullptr);
    IVF_CHECK(sphere->mesh()->vertices()->rows() > 0);
    IVF_CHECK(box->mesh() != nullptr);
    IVF_CHECK(cube->localBoundingBox().isValid());

    RenderContext::clearCurrent();
}

IVF_TEST(shape_changes_need_no_gl)
{
    auto ctx = makeContext();

    auto cube = Cube::create();
    cube->setSize(2.0f); // rebuilds the mesh
    auto box = Box::create();
    box->setSize(glm::vec3(4.0f, 1.0f, 1.0f));
    auto sphere = Sphere::create();
    sphere->setRadius(3.0);

    cube->updateVertices(); // only marks the data as changed
    cube->updateNormals();

    IVF_CHECK(cube->size() == 2.0f);
    IVF_CHECK(sphere->mesh()->vertices()->rows() > 0);

    RenderContext::clearCurrent();
}

IVF_TEST(scene_graph_is_built_without_gl)
{
    auto ctx = makeContext();

    auto scene = CompositeNode::create();
    for (int i = 0; i < 10; ++i)
    {
        auto cube = Cube::create();
        cube->setPos(glm::vec3(float(i), 0.0f, 0.0f));
        scene->add(cube);
    }
    IVF_CHECK(scene->count() == 10);

    RenderContext::clearCurrent();
}

IVF_TEST(delete_later_runs_when_the_context_is_made_current)
{
    auto ctx = RenderContext::create(nullptr);
    int runs = 0;
    ctx->deleteLater([&runs] { ++runs; });
    IVF_CHECK(runs == 0);

    ctx->makeCurrent();
    IVF_CHECK(runs == 1);
    ctx->makeCurrent(); // runs only once
    IVF_CHECK(runs == 1);

    RenderContext::clearCurrent();
}

IVF_TEST(delete_later_runs_when_the_context_is_destroyed)
{
    auto ctx = RenderContext::create(nullptr);
    int runs = 0;
    ctx->deleteLater([&runs] { ++runs; });
    ctx.reset();
    IVF_CHECK(runs == 1);
}

IVF_TEST(lifetime_token_expires_with_the_context)
{
    auto ctx = RenderContext::create(nullptr);
    std::weak_ptr<const void> token = ctx->lifetime();
    IVF_CHECK(!token.expired());
    ctx.reset();
    IVF_CHECK(token.expired());
}

IVF_TEST_MAIN()
