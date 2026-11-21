#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <stack>
#include <memory>


namespace ivf {

/**
 * @class MeshManager
 * @brief Singleton class for managing mesh state and default mesh usage.
 *
 * The MeshManager class provides a global interface for managing mesh-related state,
 * such as default OpenGL usage flags and state stack for serialization. It is implemented
 * as a singleton and supports push/pop of state using JSON for flexible state management.
 */
class MeshManager {
    friend class RenderContext; // owns and creates the manager

private:
    MeshManager();                  ///< Private constructor for singleton pattern.

    /** @brief Saved mesh state (see pushState()/popState()). */
    struct State {
        GLenum defaultMeshUsage;
    };
    std::stack<State> m_stateStack; ///< Stack for storing mesh state.
    size_t m_maxStackSize = 100;             ///< Maximum stack size to prevent memory bloat.

    GLenum m_defaultMeshUsage{GL_STATIC_DRAW}; ///< Default OpenGL usage for mesh buffers.

public:
    /**
     * @brief The MeshManager of the current RenderContext (RenderContext::current()->meshes()).
     * @return MeshManager* Pointer to the manager.
     */
    static MeshManager *instance();

    /**
     * @brief Same as instance(); kept for compatibility.
     * @return MeshManager* Pointer to the manager.
     */
    static MeshManager *create();

    /**
     * @brief Deprecated no-op: managers are owned and released by their RenderContext.
     */
    static void drop();

    /**
     * @brief Push the current mesh state onto the stack.
     */
    void pushState();

    /**
     * @brief Set the default OpenGL usage for mesh buffers.
     * @param usage OpenGL usage flag (e.g., GL_STATIC_DRAW).
     */
    void setDefaultMeshUsage(GLenum usage);

    /**
     * @brief Get the default OpenGL usage for mesh buffers.
     * @return GLenum OpenGL usage flag.
     */
    GLenum defaultMeshUsage() const;

    /**
     * @brief Pop the last mesh state from the stack.
     */
    void popState();
};

/**
 * @brief Push the current mesh state using the global MeshManager.
 */
void mmPushState();

/**
 * @brief Set the default mesh usage using the global MeshManager.
 * @param usage OpenGL usage flag.
 */
void mmDefaultMeshUsage(GLenum usage);

/**
 * @brief Get the default mesh usage from the global MeshManager.
 * @return GLenum OpenGL usage flag.
 */
GLenum mmDefaultMeshUsage();

/**
 * @brief Pop the last mesh state using the global MeshManager.
 */
void mmPopState();

/**
 * @typedef MeshManagerPtr
 * @brief Pointer type for MeshManager singleton.
 */
typedef MeshManager *MeshManagerPtr;

} // namespace ivf
