// Shadow map matrices without OpenGL: cascade splits and fitting, point light cube faces,
// and the per-node shadow flag of the render queue.

#include "test_main.h"

#include <ivf/composite_node.h>
#include <ivf/cube.h>
#include <ivf/render_queue.h>
#include <ivf/shadow_math.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

using namespace ivf;

namespace {

bool insideClip(const glm::mat4 &m, const glm::vec3 &p, float eps = 1e-3f)
{
    const glm::vec4 c = m * glm::vec4(p, 1.0f);
    const glm::vec3 ndc = glm::vec3(c) / c.w;
    return std::abs(ndc.x) <= 1.0f + eps && std::abs(ndc.y) <= 1.0f + eps && std::abs(ndc.z) <= 1.0f + eps;
}

} // namespace

IVF_TEST(near_far_from_projection)
{
    const glm::vec2 p = projectionNearFar(glm::perspective(glm::radians(45.0f), 1.5f, 0.1f, 250.0f));
    IVF_CHECK_NEAR(p.x, 0.1, 1e-4);
    IVF_CHECK_NEAR(p.y, 250.0, 0.05);
    const glm::vec2 o = projectionNearFar(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 2.0f, 30.0f));
    IVF_CHECK_NEAR(o.x, 2.0, 1e-4);
    IVF_CHECK_NEAR(o.y, 30.0, 1e-3);
}

IVF_TEST(cascade_splits_increase_to_the_far_distance)
{
    for (float lambda : {0.0f, 0.5f, 1.0f})
    {
        const auto splits = cascadeSplits(0.1f, 100.0f, 4, lambda);
        IVF_CHECK(splits.size() == 4);
        IVF_CHECK(std::is_sorted(splits.begin(), splits.end()));
        IVF_CHECK(splits.front() > 0.1f);
        IVF_CHECK(splits.back() == 100.0f);
    }
    // Logarithmic splits give the first cascade a shorter range than uniform ones
    IVF_CHECK(cascadeSplits(0.1f, 100.0f, 4, 1.0f)[0] < cascadeSplits(0.1f, 100.0f, 4, 0.0f)[0]);
    IVF_CHECK_NEAR(cascadeSplits(0.1f, 100.0f, 4, 0.0f)[1], 50.05, 1e-3);
}

IVF_TEST(slice_corners_lie_at_the_requested_distances)
{
    const glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 2.0f, 10.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 projection = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.5f, 200.0f);
    const auto corners = frustumSliceCorners(view, projection, 5.0f, 20.0f);
    for (int i = 0; i < 8; ++i)
    {
        const float depth = -(view * glm::vec4(corners[static_cast<std::size_t>(i)], 1.0f)).z;
        IVF_CHECK_NEAR(depth, i < 4 ? 5.0 : 20.0, 1e-3);
    }
}

IVF_TEST(cascade_contains_its_slice_and_the_casters)
{
    const glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 10.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0, 1, 0));
    const glm::mat4 projection = glm::perspective(glm::radians(50.0f), 1.6f, 0.1f, 100.0f);
    const glm::vec3 lightDir = glm::normalize(glm::vec3(-0.4f, -1.0f, -0.3f));
    const BoundingBox scene(glm::vec3(-50.0f, 0.0f, -50.0f), glm::vec3(50.0f, 30.0f, 50.0f));

    const auto corners = frustumSliceCorners(view, projection, 3.0f, 12.0f);
    const glm::mat4 m = fitCascade(corners, lightDir, scene, 1024);
    for (const auto &c : corners)
        IVF_CHECK(insideClip(m, c));

    // A caster high above the slice, between it and the light, is in front of the near plane
    const glm::vec3 centre = (corners[0] + corners[7]) * 0.5f;
    const glm::vec3 caster = centre - lightDir * 25.0f;
    const glm::vec4 clip = m * glm::vec4(caster, 1.0f);
    IVF_CHECK(clip.z / clip.w >= -1.0f);
}

IVF_TEST(cascades_move_in_whole_texels)
{
    const glm::mat4 projection = glm::perspective(glm::radians(50.0f), 1.6f, 0.1f, 100.0f);
    const glm::vec3 lightDir = glm::normalize(glm::vec3(0.3f, -1.0f, 0.2f));
    const BoundingBox scene(glm::vec3(-20.0f), glm::vec3(20.0f));
    const int resolution = 512;

    glm::mat4 first(1.0f);
    for (int step = 0; step < 5; ++step)
    {
        const glm::vec3 eye(0.37f * float(step), 2.0f, 10.0f);
        const glm::mat4 view = glm::lookAt(eye, eye + glm::vec3(0.0f, -0.1f, -1.0f), glm::vec3(0, 1, 0));
        const glm::mat4 m = fitCascade(frustumSliceCorners(view, projection, 0.1f, 8.0f), lightDir, scene, resolution);
        if (step == 0)
        {
            first = m;
            continue;
        }
        // The same world point lands on the texel grid shifted by whole texels
        const glm::vec4 a = first * glm::vec4(1.0f, 0.0f, -2.0f, 1.0f);
        const glm::vec4 b = m * glm::vec4(1.0f, 0.0f, -2.0f, 1.0f);
        const glm::vec2 shift = (glm::vec2(b) - glm::vec2(a)) * float(resolution) * 0.5f;
        IVF_CHECK_NEAR(shift.x, std::round(shift.x), 2e-2);
        IVF_CHECK_NEAR(shift.y, std::round(shift.y), 2e-2);
    }
}

IVF_TEST(point_light_faces_cover_all_directions)
{
    const glm::vec3 light(1.0f, 2.0f, -3.0f);
    const auto matrices = pointShadowMatrices(light, 0.05f, 20.0f, 1.05f);
    const glm::vec3 directions[] = {{1, 0.3f, -0.2f}, {-1, 0.9f, 0.5f}, {0.2f, 1, -0.7f},
                                    {0.1f, -1, 0.2f}, {-0.6f, 0.4f, 1},  {0.95f, -0.9f, -1}};
    for (const auto &d : directions)
    {
        const glm::vec3 p = light + glm::normalize(d) * 7.0f;
        const int face = cubeFace(p - light);
        IVF_CHECK(insideClip(matrices[static_cast<std::size_t>(face)], p));
        // and not in front of the opposite face
        const glm::vec4 c = matrices[static_cast<std::size_t>(face ^ 1)] * glm::vec4(p, 1.0f);
        IVF_CHECK(c.w < 0.0f);
    }
    IVF_CHECK(cubeFace({0.0f, 0.0f, -2.0f}) == 5);
    IVF_CHECK(cubeFace({0.0f, 3.0f, 1.0f}) == 2);
}

IVF_TEST(cast_shadows_flag_is_inherited)
{
    auto group = CompositeNode::create();
    auto a = Cube::create();
    auto b = Cube::create();
    group->add(a);
    group->add(b);
    b->setCastShadows(false);

    RenderQueue queue;
    group->collect(queue, CollectState{});
    IVF_CHECK(queue.size() == 2);
    IVF_CHECK(queue.items()[0].castShadows);
    IVF_CHECK(!queue.items()[1].castShadows);

    group->setCastShadows(false);
    RenderQueue hidden;
    group->collect(hidden, CollectState{});
    for (const auto &item : hidden.items())
        IVF_CHECK(!item.castShadows);
}

IVF_TEST(filter_names)
{
    IVF_CHECK(std::string(shadowFilterName(ShadowFilter::Pcss)) == "PCSS");
    IVF_CHECK(ShadowFilterCount == 3);
}

IVF_TEST_MAIN()
