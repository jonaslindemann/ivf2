#include <ivf/axis.h>

#include <ivf/light_manager.h>

using namespace ivf;

Axis::Axis(GLfloat size)
	:m_size(size)
{
    this->newMesh(6);
    this->doSetup();
}

Axis::~Axis()
{
    
}

std::shared_ptr<Axis> ivf::Axis::create(GLfloat size)
{
    return std::make_shared<Axis>(size);
}

void Axis::setSize(GLfloat size)
{
    m_size = size;
    this->refresh();
}

GLfloat Axis::size()
{
    return m_size;
}

void ivf::Axis::doSetup()
{
    mesh()->begin(GL_LINES);

    mesh()->vertex3d(0.0, 0.0, 0.0);
    mesh()->color3f(1.0, 0.0, 0.0);
    mesh()->vertex3d(m_size, 0.0, 0.0);
    mesh()->color3f(1.0, 0.0, 0.0);

    mesh()->vertex3d(0.0, 0.0, 0.0);
    mesh()->color3f(0.0, 1.0, 0.0);
    mesh()->vertex3d(0.0, m_size, 0.0);
    mesh()->color3f(0.0, 1.0, 0.0);

    mesh()->vertex3d(0.0, 0.0, 0.0);
    mesh()->color3f(0.0, 0.0, 1.0);
    mesh()->vertex3d(0.0, 0.0, m_size);
    mesh()->color3f(0.0, 0.0, 1.0);

    mesh()->end();
}

void Axis::doPreDraw()
{
	LightManager::instance()->saveState();
	LightManager::instance()->disableLighting();
}

void Axis::doPostDraw()
{
	LightManager::instance()->restoreState();
}

