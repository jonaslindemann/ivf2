#include <ivf/mesh_manager.h>
#include <ivf/render_context.h>

using namespace ivf;
using namespace std;

// The manager lives in the current RenderContext (see ivf/render_context.h).
MeshManager *MeshManager::instance()
{
    return RenderContext::currentOrFallback()->meshes();
}

MeshManager *MeshManager::create()
{
    return instance();
}

void MeshManager::drop()
{
    // Managers are released together with their RenderContext.
}


MeshManager::MeshManager()
{}

void ivf::MeshManager::pushState()
{
    if (m_stateStack.size() >= m_maxStackSize)
    {
        m_stateStack.pop();
    }

    m_stateStack.push(State{m_defaultMeshUsage});
}

void MeshManager::setDefaultMeshUsage(GLenum usage)
{
    m_defaultMeshUsage = usage;
}

GLenum MeshManager::defaultMeshUsage() const
{
    return m_defaultMeshUsage;
}

void ivf::MeshManager::popState()
{
    if (!m_stateStack.empty())
    {
        m_defaultMeshUsage = m_stateStack.top().defaultMeshUsage;
        m_stateStack.pop();
    }
}

void ivf::mmDefaultMeshUsage(GLenum usage)
{
    MeshManager::instance()->setDefaultMeshUsage(usage);
}

GLenum ivf::mmDefaultMeshUsage()
{
    // Meshes can be built before any context exists (they are uploaded on first draw);
    // without a current context use the default instead of the fallback context.
    if (RenderContext::current() == nullptr)
        return GL_STATIC_DRAW;
    return MeshManager::instance()->defaultMeshUsage();
}

void ivf::mmPushState()
{
    MeshManager::instance()->pushState();
}

void ivf::mmPopState()
{
    MeshManager::instance()->popState();
}
