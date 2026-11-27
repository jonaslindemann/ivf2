// Package consumer test for ivfphysics: links without physics headers or libraries of its own.

#include <ivf/cube.h>
#include <ivfphysics/ivfphysics.h>

int main()
{
    auto world = ivfphysics::PhysicsWorld::create();
    auto cube = ivf::Cube::create();
    cube->setPos({0.0f, 2.0f, 0.0f});
    world->addBody(cube, ivfphysics::Shape::box(glm::vec3(0.5f)));
    for (int i = 0; i < 60; ++i)
        world->step(1.0 / 60.0);
    return cube->pos().y < 2.0f ? 0 : 1;
}
