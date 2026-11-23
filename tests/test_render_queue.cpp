// Collecting a scene into a RenderQueue (roadmap phase 2, render queue step 2). Collection
// needs no OpenGL context: it only walks the scene graph and records draw items.

#include "test_main.h"

#include <ivf/box.h>
#include <ivf/capped_cone.h>
#include <ivf/capped_cylinder.h>
#include <ivf/composite_node.h>
#include <ivf/cone.h>
#include <ivf/cylinder.h>
#include <ivf/dodecahedron.h>
#include <ivf/plane.h>
#include <ivf/rounded_box.h>
#include <ivf/sphere.h>
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

IVF_TEST(items_have_world_bounds)
{
    Context c;
    auto scene = CompositeNode::create();
    auto group = CompositeNode::create();
    group->setPos({10.0f, 0.0f, 0.0f});
    auto cube = Cube::create(); // unit cube: local bounds -0.5..0.5
    cube->setPos({0.0f, 2.0f, 0.0f});
    group->add(cube);
    scene->add(group);
    auto other = Cube::create();
    other->setPos({-3.0f, 0.0f, 0.0f});
    scene->add(other);

    auto queue = collect(scene);
    IVF_CHECK(queue.size() == 2);
    const BoundingBox &b = queue.items()[0].bounds;
    IVF_CHECK(b.isValid());
    IVF_CHECK_NEAR(b.min().x, 9.5, 1e-5);
    IVF_CHECK_NEAR(b.max().x, 10.5, 1e-5);
    IVF_CHECK_NEAR(b.min().y, 1.5, 1e-5);

    // Scene box used for automatic shadow framing
    const BoundingBox all = queue.bounds();
    IVF_CHECK(all.isValid());
    IVF_CHECK_NEAR(all.min().x, -3.5, 1e-5);
    IVF_CHECK_NEAR(all.max().x, 10.5, 1e-5);
}

namespace {

// The local bounding box must enclose the node's vertices exactly (culling relies on it)
bool boundsMatchVertices(MeshNode &node)
{
    BoundingBox extent;
    for (const auto &mesh : node.meshes())
        for (GLuint i = 0; i < mesh->vertices()->rows(); ++i)
            extent.add(mesh->vertices()->vertex(i));
    const BoundingBox b = node.localBoundingBox();
    if (!b.isValid() || !extent.isValid())
        return false;
    const glm::vec3 dmin = glm::abs(b.min() - extent.min());
    const glm::vec3 dmax = glm::abs(b.max() - extent.max());
    return glm::max(glm::max(dmin.x, dmin.y), glm::max(dmin.z, glm::max(dmax.x, glm::max(dmax.y, dmax.z)))) < 1e-4f;
}

} // namespace

IVF_TEST(primitive_bounds_enclose_their_vertices)
{
    Context c;
    IVF_CHECK(boundsMatchVertices(*Cube::create()));
    IVF_CHECK(boundsMatchVertices(*Box::create(glm::vec3(1.0f, 2.0f, 3.0f))));
    IVF_CHECK(boundsMatchVertices(*RoundedBox::create(glm::vec3(1.0f, 2.0f, 3.0f))));
    IVF_CHECK(boundsMatchVertices(*Plane::create(4.0, 2.0)));
    IVF_CHECK(boundsMatchVertices(*Sphere::create(1.5)));
    IVF_CHECK(boundsMatchVertices(*Cone::create(0.5, 2.0)));
    IVF_CHECK(boundsMatchVertices(*Cylinder::create(0.5, 2.0)));
    IVF_CHECK(boundsMatchVertices(*CappedCone::create(0.5, 2.0)));
    IVF_CHECK(boundsMatchVertices(*CappedCylinder::create(0.5, 2.0)));
    IVF_CHECK(boundsMatchVertices(*Dodecahedron::create(1.0)));
}

IVF_TEST(layers_follow_materials)
{
    Context c;
    auto opaque = Material::create();
    auto glass = Material::create();
    glass->setAlpha(0.5f);

    auto scene = CompositeNode::create();
    auto a = Cube::create();
    a->setMaterial(opaque);
    auto b = Cube::create();
    b->setMaterial(glass);
    auto group = CompositeNode::create();
    group->setMaterial(glass);
    auto inherits = Cube::create(); // inherits the group's transparent material
    group->add(inherits);
    auto forced = Cube::create();
    forced->setMaterial(glass);
    forced->setRenderLayer(RenderLayer::Overlay); // explicit layer wins
    scene->add(a);
    scene->add(b);
    scene->add(group);
    scene->add(forced);

    auto queue = collect(scene);
    IVF_CHECK(queue.size() == 4);
    IVF_CHECK(queue.items()[0].layer == RenderLayer::Opaque);
    IVF_CHECK(queue.items()[1].layer == RenderLayer::Transparent);
    IVF_CHECK(queue.items()[2].layer == RenderLayer::Transparent);
    IVF_CHECK(queue.items()[3].layer == RenderLayer::Overlay);
}

IVF_TEST(sort_groups_layers_and_orders_by_depth)
{
    Context c;
    auto opaque = Material::create();
    auto glass = Material::create();
    glass->setAlpha(0.5f);

    // Added in a mixed order; the camera looks from +Z
    auto scene = CompositeNode::create();
    auto makeCube = [&](float z, const MaterialPtr &material) {
        auto cube = Cube::create();
        cube->setPos({0.0f, 0.0f, z});
        cube->setMaterial(material);
        scene->add(cube);
        return cube.get();
    };
    Node *glassNear = makeCube(5.0f, glass);
    Node *opaqueFar = makeCube(-5.0f, opaque);
    Node *glassFar = makeCube(-5.0f, glass);
    Node *opaqueNear = makeCube(5.0f, opaque);
    auto sky = Cube::create();
    sky->setRenderLayer(RenderLayer::Background);
    scene->add(sky);

    auto queue = collect(scene);
    queue.sortForView({0.0f, 0.0f, 20.0f});
    IVF_CHECK(queue.size() == 5);
    IVF_CHECK(queue.items()[0].node == sky.get()); // background first
    IVF_CHECK(queue.items()[1].node == opaqueNear); // opaque front to back
    IVF_CHECK(queue.items()[2].node == opaqueFar);
    IVF_CHECK(queue.items()[3].node == glassFar); // transparent back to front
    IVF_CHECK(queue.items()[4].node == glassNear);
}

namespace {

// The winding Mesh::clockwise() reports must make the normals point out of the front faces
bool windingMatchesNormals(MeshNode &node)
{
    for (const auto &mesh : node.meshes())
    {
        auto verts = mesh->vertices();
        auto normals = mesh->normals();
        auto indices = mesh->indices();
        if (!indices)
            continue;
        long agree = 0;
        for (GLuint i = 0; i < indices->rows(); ++i)
        {
            const GLuint i0 = indices->at(i, 0), i1 = indices->at(i, 1), i2 = indices->at(i, 2);
            glm::vec3 face = glm::cross(verts->vertex(i1) - verts->vertex(i0), verts->vertex(i2) - verts->vertex(i0));
            if (mesh->clockwise())
                face = -face;
            const float d = glm::dot(face, normals->normal(i0) + normals->normal(i1) + normals->normal(i2));
            agree += d > 0.0f ? 1 : (d < 0.0f ? -1 : 0);
        }
        if (agree <= 0)
            return false;
    }
    return true;
}

} // namespace

IVF_TEST(mesh_winding_matches_normals)
{
    Context c;
    IVF_CHECK(windingMatchesNormals(*Cube::create()));
    IVF_CHECK(windingMatchesNormals(*Sphere::create(1.0)));
    IVF_CHECK(windingMatchesNormals(*Box::create()));
    IVF_CHECK(windingMatchesNormals(*Cone::create(0.5, 2.0)));
    IVF_CHECK(windingMatchesNormals(*Plane::create(2.0, 2.0)));
    // Generator meshes are mirrored by the Y/Z swap, so they are clockwise
    IVF_CHECK(Sphere::create(1.0)->mesh()->clockwise());
}

IVF_TEST_MAIN()
