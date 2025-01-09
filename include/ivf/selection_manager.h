#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

class SelectionManager {
private:
    SelectionManager();
    static SelectionManager *m_instance;

    uint32_t m_objectId;
    uint32_t m_savedObjectId;
    GLint m_objectIdId;

    bool m_selectionRendering;
    bool m_savedSelectionRendering;
    GLint m_selectionRenderingId;

public:
    static SelectionManager *instance()
    {
        if (!m_instance)
            m_instance = new SelectionManager();

        return m_instance;
    }

    static SelectionManager *create()
    {
        return instance();
    }

    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    void setObjectId(uint32_t objectId);
    uint32_t objectId();

    void setSelectionRendering(bool selectionRendering);
    bool selectionRendering();

    void apply();

    void saveState();
    void restoreState();
};

typedef SelectionManager *SelectionManagerPtr;
}; // namespace ivf
