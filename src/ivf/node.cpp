#include <ivf/node.h>

#include <ivf/selection_manager.h>

using namespace ivf;

void Node::draw()
{
    doPreDraw();
    if (m_visible)
        doDraw();
    doPostDraw();
}

void ivf::Node::drawSelection()
{
    if (m_visible)
        doDrawSelection();
}

void ivf::Node::setMaterial(std::shared_ptr<Material> material)
{
    m_material = material;
}

std::shared_ptr<Material> ivf::Node::material()
{
    return m_material;
}

void ivf::Node::setUseMaterial(bool flag)
{
    m_useMaterial = flag;
}

void Node::setTexture(std::shared_ptr<Texture> texture)
{
    m_texture = texture;
    if (m_material != nullptr)
        m_material->setUseTexture(true);

    m_useTexture = true;
}

std::shared_ptr<Texture> ivf::Node::texture()
{
    return m_texture;
}

bool ivf::Node::useMaterial()
{
    return m_useMaterial;
}

void ivf::Node::setUseTexture(bool flag)
{
    m_useTexture = flag;
}

bool ivf::Node::useTexture()
{
    return m_useTexture;
}

void ivf::Node::setVisible(bool flag)
{
    m_visible = flag;
}

bool ivf::Node::visible() const
{
    return m_visible;
}

void ivf::Node::setObjectId(uint32_t objectId)
{
    m_objectId = objectId;
}

uint32_t ivf::Node::objectId() const
{
    return m_objectId;
}

uint32_t ivf::Node::enumerateIds(uint32_t startId)
{
    return this->doEnumerateIds(startId);
}

void ivf::Node::accept(NodeVisitor *visitor)
{
    visitor->visit(this);
}

void Node::doPreDraw()
{
    SelectionManager::instance()->setObjectId(m_objectId);

    if ((m_material != nullptr) && (m_useMaterial))
        m_material->apply();
    if ((m_texture != nullptr) && (m_useTexture))
        m_texture->bind();
}

void ivf::Node::doDraw()
{}

void ivf::Node::doPostDraw()
{
    m_texture->unbind();
}

void ivf::Node::doDrawSelection()
{}

uint32_t ivf::Node::doEnumerateIds(uint32_t startId)
{
    m_objectId = startId;
    return startId + 1;
}
