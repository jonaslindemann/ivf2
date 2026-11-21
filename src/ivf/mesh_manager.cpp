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

    nlohmann::json state;
    state["defaultMeshUsage"] = m_defaultMeshUsage;
    m_stateStack.push(state);
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
        nlohmann::json state = m_stateStack.top();
        m_stateStack.pop();
        if (state.contains("defaultMeshUsage"))
        {
            m_defaultMeshUsage = state["defaultMeshUsage"].get<GLenum>();
        }
    }
}

void ivf::mmDefaultMeshUsage(GLenum usage)
{
    MeshManager::instance()->setDefaultMeshUsage(usage);
}

GLenum ivf::mmDefaultMeshUsage()
{
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
