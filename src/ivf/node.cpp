#include <ivf/node.h>

using namespace ivf;

void Node::draw()
{
    doPreDraw();
    if (m_visible)
        doDraw();
    doPostDraw();
}

void ivf::Node::setMaterial(std::shared_ptr<Material> material)
{
    m_material = material;
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

void Node::doPreDraw()
{
    if ((m_material != nullptr) && (m_useMaterial))
        m_material->apply();
    if ((m_texture != nullptr) && (m_useTexture))
        m_texture->bind();
}

void ivf::Node::doDraw()
{
}

void ivf::Node::doPostDraw()
{
    m_texture->unbind();
}
