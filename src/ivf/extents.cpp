#include <ivf/extents.h>

#include <ivf/light_manager.h>

using namespace ivf;

Extents::Extents(BoundingBox bbox) : m_bbox(bbox)   
{
    this->newMesh(24, 12);
    this->doSetup();
    this->setName("Extents");
}

std::shared_ptr<Extents> ivf::Extents::create(BoundingBox bbox)
{
    return std::make_shared<Extents>(bbox);
}

void Extents::setBoundingBox(BoundingBox bbox)
{
    m_bbox = bbox;
    this->refresh();
}

BoundingBox Extents::bbox()
{
    return m_bbox;
}

void Extents::doSetup()
{
    this->clear();
    this->newMesh(24, 0); // 12 lines * 2 vertices = 24 vertices

    mesh()->begin(GL_LINES);

    // Bottom rectangle (z = min)
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.min().y, m_bbox.min().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.min().y, m_bbox.min().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.min().y, m_bbox.min().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.max().y, m_bbox.min().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.max().y, m_bbox.min().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.max().y, m_bbox.min().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.max().y, m_bbox.min().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.min().y, m_bbox.min().z);

    // Top rectangle (z = max)
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.min().y, m_bbox.max().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.min().y, m_bbox.max().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.min().y, m_bbox.max().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.max().y, m_bbox.max().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.max().y, m_bbox.max().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.max().y, m_bbox.max().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.max().y, m_bbox.max().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.min().y, m_bbox.max().z);

    // Vertical edges connecting bottom and top
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.min().y, m_bbox.min().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.min().y, m_bbox.max().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.min().y, m_bbox.min().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.min().y, m_bbox.max().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.max().y, m_bbox.min().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.max().x, m_bbox.max().y, m_bbox.max().z);

    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.max().y, m_bbox.min().z);
    mesh()->color3f(1.0f, 1.0f, 1.0f);
    mesh()->vertex3d(m_bbox.min().x, m_bbox.max().y, m_bbox.max().z);

    mesh()->end();
}

void ivf::Extents::setupProperties()
{
    MeshNode::setupProperties();
}

void ivf::Extents::onPropertyChanged(const std::string &name)
{}

void Extents::doPreDraw()
{
    MeshNode::doPreDraw();
    LightManager::instance()->saveState();
    LightManager::instance()->disableLighting();
}

void Extents::doPostDraw()
{
    LightManager::instance()->restoreState();
    MeshNode::doPostDraw();
}
