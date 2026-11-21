#include "test_main.h"

#include <ivfmath/spline.h>

#include <glm/glm.hpp>

IVF_TEST(spline_empty_is_origin)
{
    auto s = ivfmath::Spline::create();
    IVF_CHECK(s->size() == 0);
    IVF_CHECK_NEAR(s->totalLength(), 0.0, 1e-6);
    IVF_CHECK(s->positionByT(0.5f) == glm::vec3(0.0f));
}

IVF_TEST(spline_passes_through_control_points)
{
    auto s = ivfmath::Spline::create();
    s->addPoint({0, 0, 0});
    s->addPoint({1, 2, 0});
    s->addPoint({3, 1, 1});
    s->addPoint({4, 0, 2});

    for (int i = 0; i < 4; ++i)
    {
        auto p = s->positionByT(float(i));
        IVF_CHECK(glm::length(p - (i == 0   ? glm::vec3(0, 0, 0)
                                   : i == 1 ? glm::vec3(1, 2, 0)
                                   : i == 2 ? glm::vec3(3, 1, 1)
                                            : glm::vec3(4, 0, 2))) < 1e-5f);
    }
}

IVF_TEST(spline_straight_line_length)
{
    auto s = ivfmath::Spline::create();
    for (int i = 0; i < 4; ++i)
        s->addPoint({float(i), 0, 0});

    IVF_CHECK_NEAR(s->totalLength(), 3.0, 0.05);

    auto mid = s->positionByDistance(1.5f);
    IVF_CHECK_NEAR(mid.x, 1.5, 0.05);
    IVF_CHECK_NEAR(mid.y, 0.0, 1e-5);
}

IVF_TEST(spline_clamps_parameter)
{
    auto s = ivfmath::Spline::create();
    s->addPoint({0, 0, 0});
    s->addPoint({1, 0, 0});
    IVF_CHECK(glm::length(s->positionByT(-5.0f) - glm::vec3(0, 0, 0)) < 1e-6f);
    IVF_CHECK(glm::length(s->positionByT(5.0f) - glm::vec3(1, 0, 0)) < 1e-6f);
}

IVF_TEST_MAIN()
