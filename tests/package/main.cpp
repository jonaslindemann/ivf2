// Compiles against the public headers of every ivf2 library and links them.
// It deliberately makes no OpenGL or windowing calls, so it can run on
// headless CI machines.

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/version.h>
#include <ivfmath/spline.h>
#include <ivfui/glfw_application.h>
#include <ivfui/glfw_window.h>
#include <generator/generator.hpp>

#include <iostream>

int main()
{
    std::cout << ivf::version_long_name << " " << ivf::version_string << "\n";

    auto spline = ivfmath::Spline::create();
    spline->addPoint({0, 0, 0});
    spline->addPoint({1, 0, 0});

    generator::SphereMesh sphere(1.0, 8, 4);

    return (spline->size() == 2 && generator::count(sphere.vertices()) > 0) ? 0 : 1;
}
