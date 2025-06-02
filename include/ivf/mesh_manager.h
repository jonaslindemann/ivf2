#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <stack>
#include <memory>

#include <nlohmann/json.hpp>

namespace ivf {

class MeshManager {
private:
    MeshManager();
    static MeshManager *m_instance;

    std::stack<nlohmann::json> m_stateStack;
    size_t m_maxStackSize = 100; // Prevent memory bloat

    GLenum m_defaultMeshUsage{GL_STATIC_DRAW};

public:
    static MeshManager *instance()
    {
        if (!m_instance)
        {
            if (!m_instance)
                m_instance = new MeshManager();
        }
        return m_instance;
    }

    static MeshManager *create()
    {
        if (!m_instance)
        {
            if (!m_instance)
                m_instance = new MeshManager();
        }
        return m_instance;
    }

    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    void pushState();

    void setDefaultMeshUsage(GLenum usage);
    GLenum defaultMeshUsage() const;

    void popState();
};

void mmPushState();

void mmDefaultMeshUsage(GLenum usage);
GLenum mmDefaultMeshUsage();

void mmPopState();

typedef MeshManager *MeshManagerPtr;

} // namespace ivf
