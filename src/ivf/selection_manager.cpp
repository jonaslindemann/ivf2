#include <ivf/selection_manager.h>
#include <ivf/render_context.h>

#include <ivf/shader_manager.h>


using namespace ivf;

// The manager lives in the current RenderContext (see ivf/render_context.h).
SelectionManager *SelectionManager::instance()
{
    return RenderContext::currentOrFallback()->selection();
}

SelectionManager *SelectionManager::create()
{
    return instance();
}

void SelectionManager::drop()
{
    // Managers are released together with their RenderContext.
}


SelectionManager::SelectionManager()
    : m_objectId(0), m_objectIdId(-1), m_selectionRendering(false), m_selectionRenderingId(-1), m_savedObjectId(0),
      m_savedSelectionRendering(false)
{
    m_objectIdId = ShaderManager::instance()->currentProgram()->uniformLoc("objectId");
    m_selectionRenderingId = ShaderManager::instance()->currentProgram()->uniformLoc("selectionRendering");
}

void ivf::SelectionManager::setObjectId(uint32_t objectId)
{
    m_objectId = objectId;
    ShaderManager::instance()->currentProgram()->uniformUInt(m_objectIdId, m_objectId);
}

uint32_t ivf::SelectionManager::objectId()
{
    return m_objectId;
}

void ivf::SelectionManager::setSelectionRendering(bool selectionRendering)
{
    m_selectionRendering = selectionRendering;
    ShaderManager::instance()->currentProgram()->uniformBool(m_selectionRenderingId, m_selectionRendering);
}

bool ivf::SelectionManager::selectionRendering()
{
    return m_selectionRendering;
}

void ivf::SelectionManager::apply()
{
    ShaderManager::instance()->currentProgram()->uniformUInt(m_objectIdId, m_objectId);
    ShaderManager::instance()->currentProgram()->uniformBool(m_selectionRenderingId, m_selectionRendering);
}

void SelectionManager::saveState()
{
    m_savedObjectId = m_objectId;
    m_savedSelectionRendering = m_selectionRendering;
}

void SelectionManager::restoreState()
{
    m_objectId = m_savedObjectId;
    m_selectionRendering = m_savedSelectionRendering;
    apply();
}

void SelectionManager::refreshForProgram()
{
    auto prog = ShaderManager::instance()->currentProgram();
    m_objectIdId = prog->uniformLoc("objectId");
    m_selectionRenderingId = prog->uniformLoc("selectionRendering");
}
