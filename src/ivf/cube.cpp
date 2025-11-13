#include <ivf/cube.h>

using namespace ivf;

Cube::Cube(GLfloat size) : m_size(size)
{
    this->newMesh(24, 12);
    this->doSetup();
    this->setName("Cube");
}

Cube::~Cube()
{}

std::shared_ptr<Cube> ivf::Cube::create()
{
    return std::make_shared<Cube>();
}

void Cube::setSize(GLfloat size)
{
    m_size = size;
    this->refresh();
}

GLfloat Cube::size()
{
    return m_size;
}

void Cube::doSetup()
{
    //   y  o--------o
    //   ^ /|       /|
    //   |/ |     2/ |
    //  3o--------o  |
    //   |  o-----|--o
    //   | /      | /
    //   |/       |/
    //   o--------o --> x
    //   0        1

    this->clear();
    this->newMesh(24, 12);

    double n = m_size / 2.0;

    // Set bounding box for the cube
    setLocalBoundingBox(BoundingBox(glm::vec3(-n, -n, -n), glm::vec3(n, n, n)));

    mesh()->begin(GL_TRIANGLES);
    mesh()->vertex3d(-n, -n, n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);
    mesh()->vertex3d(n, -n, n);
    mesh()->color3f(1.0f, 0.0f, 1.0f);
    mesh()->vertex3d(n, n, n);
    mesh()->color3f(1.0f, 1.0f, 0.0f);
    mesh()->vertex3d(-n, n, n);
    mesh()->color3f(1.0f, 1.0f, 1.0f);

    mesh()->index3i(0, 1, 2); // front
    mesh()->index3i(0, 2, 3);

    mesh()->vertex3d(-n, -n, -n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(n, -n, -n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(n, n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(-n, n, -n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(4, 6, 5); // back
    mesh()->index3i(4, 7, 6);

    mesh()->vertex3d(-n, -n, n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(-n, n, n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(-n, n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(-n, -n, -n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(8, 9, 10);
    mesh()->index3i(8, 10, 11); // left

    mesh()->vertex3d(n, -n, n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(n, -n, -n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(n, n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(n, n, n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(12, 13, 14);
    mesh()->index3i(12, 14, 15); // right

    mesh()->vertex3d(-n, n, n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(n, n, n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(n, n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(-n, n, -n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(16, 17, 18);
    mesh()->index3i(16, 18, 19); // right

    mesh()->vertex3d(-n, -n, n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(-n, -n, -n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(n, -n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(n, -n, n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(20, 21, 22);
    mesh()->index3i(20, 22, 23); // right

    mesh()->end();
}

void ivf::Cube::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Size", &m_size, "Geometry");
}

void ivf::Cube::onPropertyChanged(const std::string &name)
{}
