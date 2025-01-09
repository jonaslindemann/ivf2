#include <ivf/buffer_selection.h>
#include <ivf/node_visitor.h>
#include <ivf/selection_manager.h>

#include <iostream>

using namespace ivf;

BufferSelection::BufferSelection(CompositeNodePtr scene)
    : m_fbo(0), m_colorTexture(0), m_scene(scene), m_depthRenderBuffer(0), m_width(0), m_height(0)
{}

BufferSelection::~BufferSelection()
{
    clear();
}

void BufferSelection::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
}

void BufferSelection::clear()
{
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_colorTexture);
    glDeleteRenderbuffers(1, &m_depthRenderBuffer);
    m_nodeMap.clear();
}

std::shared_ptr<BufferSelection> BufferSelection::create(CompositeNodePtr scene)
{
    return std::make_shared<BufferSelection>(scene);
}

void BufferSelection::initialize(int width, int height)
{
    m_width = width;
    m_height = height;

    m_scene->enumerateIds(0);

    MapVisitor mapVisitor;
    m_scene->accept(&mapVisitor);
    m_nodeMap = mapVisitor.takeMap();

    std::cout << "BufferSelection::initialize: " << m_nodeMap.size() << " nodes in scene" << std::endl;

    glGenFramebuffers(1, &m_fbo);
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glGenRenderbuffers(1, &m_depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
}

Node *BufferSelection::nodeFromId(unsigned int objectId)
{
    auto it = m_nodeMap.find(objectId);
    if (it != m_nodeMap.end())
        return it->second;
    else
        return nullptr;
}

Node *BufferSelection::nodeAtPixel(int x, int y)
{
    unsigned int id = idAtPixel(x, y);
    return nodeFromId(id);
}

void BufferSelection::begin()
{
    SelectionManager::instance()->setSelectionRendering(true);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

unsigned int BufferSelection::idAtPixel(int x, int y)
{
    unsigned char pixel[3];

    glReadPixels(x, m_height - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

    uint32_t id = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);

    return id;
}

void BufferSelection::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    SelectionManager::instance()->setSelectionRendering(false);
}
