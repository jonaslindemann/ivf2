// Collecting a scene into a RenderQueue (roadmap phase 2, render queue step 2). Collection
// needs no OpenGL context: it only walks the scene graph and records draw items.

#include "test_main.h"

#include <ivf/composite_node.h>
#include <ivf/cube.h>
#include <ivf/instance_node.h>
#include <ivf/material.h>
#include <ivf/render_context.h>
#include <ivf/render_queue.h>

using namespace ivf;

namespace {

struct Context {
    Context() : ctx(RenderContext::create(nullptr))
    {
        ctx->makeCurrent();
    }
    ~Context()
    {
        RenderContext::clearCurrent();
    }
    std::shared_ptr<RenderContext> ctx;
};

RenderQueue collect(const NodePtr &root)
{
    RenderQueue queue;
    root->collect(queue, CollectState{});
    return queue;
}

} // namespace

IVF_TEST(leaves_become_items_in_scene_order)
{
    Context c;
    auto scene = CompositeNode::create();
    auto a = Cube::create();
    auto b = Cube::create();
    auto group = CompositeNode::create();
    auto d = Cube::create();
    scene->add(a);
    scene->add(group);
    group->add(d);
    scene->add(b);

    auto queue = collect(scene);
    IVF_CHECK(queue.size() == 3); // containers add no items
    IVF_CHECK(queue.items()[0].node == a.get());
    IVF_CHECK(queue.items()[1].node == d.get());
    IVF_CHECK(queue.items()[2].node == b.get());
}

IVF_TEST(world_matrices_accumulate_transforms)
{
    Context c;
    auto scene = CompositeNode::create();
    auto group = CompositeNode::create();
    group->setPos({1.0f, 0.0f, 0.0f});
    auto cube = Cube::create();
    cube->setPos({0.0f, 2.0f, 0.0f});
    scene->add(group);
    group->add(cube);

    auto queue = collect(scene);
    IVF_CHECK(queue.size() == 1);
    const DrawItem &item = queue.items()[0];
    // Parent world: the group's translation; world: group and cube translations
    IVF_CHECK_NEAR(item.parentWorld[3][0], 1.0, 1e-6);
    IVF_CHECK_NEAR(item.parentWorld[3][1], 0.0, 1e-6);
    IVF_CHECK_NEAR(item.world[3][0], 1.0, 1e-6);
    IVF_CHECK_NEAR(item.world[3][1], 2.0, 1e-6);
}

IVF_TEST(invisible_nodes_and_groups_are_skipped)
{
    Context c;
    auto scene = CompositeNode::create();
    auto hidden = Cube::create();
    hidden->setVisible(false);
    auto hiddenGroup = CompositeNode::create();
    hiddenGroup->setVisible(false);
    hiddenGroup->add(Cube::create());
    auto shown = Cube::create();
    scene->add(hidden);
    scene->add(hiddenGroup);
    scene->add(shown);

    auto queue = collect(scene);
    IVF_CHECK(queue.size() == 1);
    IVF_CHECK(queue.items()[0].node == shown.get());
}

IVF_TEST(nearest_material_wins)
{
    Context c;
    auto red = Material::create();
    auto blue = Material::create();

    auto scene = CompositeNode::create();
    auto group = CompositeNode::create();
    group->setMaterial(red);
    auto sphereLike = Cube::create(); // no material: inherits red
    auto cube = Cube::create();       // own material: blue
    cube->setMaterial(blue);
    auto cone = Cube::create(); // after a sibling with its own material: still red
    group->add(sphereLike);
    group->add(cube);
    group->add(cone);
    scene->add(group);
    auto outside = Cube::create(); // no material anywhere
    scene->add(outside);

    auto queue = collect(scene);
    IVF_CHECK(queue.size() == 4);
    IVF_CHECK(queue.items()[0].material == red);
    IVF_CHECK(queue.items()[1].material == red); // inherited value; the node's own blue is used when drawn
    IVF_CHECK(queue.items()[2].material == red);
    IVF_CHECK(queue.items()[3].material == nullptr);
}

IVF_TEST(instances_use_their_own_transform)
{
    Context c;
    auto shared = Cube::create();
    auto scene = CompositeNode::create();
    for (int i = 0; i < 3; ++i)
    {
        auto instance = InstanceNode::create();
        instance->setNode(shared);
        instance->setPos({float(i) * 10.0f, 0.0f, 0.0f});
        scene->add(instance);
    }

    auto queue = collect(scene);
    IVF_CHECK(queue.size() == 3);
    for (int i = 0; i < 3; ++i)
    {
        IVF_CHECK(queue.items()[i].node == shared.get());
        IVF_CHECK_NEAR(queue.items()[i].world[3][0], i * 10.0, 1e-5);
    }
}

IVF_TEST_MAIN()
