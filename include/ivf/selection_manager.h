#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class SelectionManager
 * @brief Singleton class for managing object selection and selection rendering state.
 *
 * The SelectionManager class provides a global interface for managing selection state,
 * such as the currently selected object ID and whether selection rendering is active.
 * It supports saving and restoring selection state, and applies selection parameters
 * to the rendering context or shaders as needed.
 */
class SelectionManager {
    friend class RenderContext; // owns and creates the manager

private:
    SelectionManager();                  ///< Private constructor for singleton pattern.

    uint32_t m_objectId;      ///< Current object ID for selection.
    uint32_t m_savedObjectId; ///< Saved object ID for state restoration.
    GLint m_objectIdId;       ///< Shader uniform location for object ID.

    bool m_selectionRendering;      ///< Whether selection rendering is active.
    bool m_savedSelectionRendering; ///< Saved selection rendering state.
    GLint m_selectionRenderingId;   ///< Shader uniform location for selection rendering.

public:
    /**
     * @brief The SelectionManager of the current RenderContext (RenderContext::current()->selection()).
     * @return SelectionManager* Pointer to the manager.
     */
    static SelectionManager *instance();

    /**
     * @brief Same as instance(); kept for compatibility.
     * @return SelectionManager* Pointer to the manager.
     */
    static SelectionManager *create();

    /**
     * @brief Deprecated no-op: managers are owned and released by their RenderContext.
     */
    static void drop();

    /**
     * @brief Set the current object ID for selection.
     * @param objectId Object ID value.
     */
    void setObjectId(uint32_t objectId);

    /**
     * @brief Get the current object ID for selection.
     * @return uint32_t Object ID value.
     */
    uint32_t objectId() const;

    /**
     * @brief Enable or disable selection rendering.
     * @param selectionRendering True to enable, false to disable.
     */
    void setSelectionRendering(bool selectionRendering);

    /**
     * @brief Check if selection rendering is enabled.
     * @return bool True if selection rendering is enabled.
     */
    bool selectionRendering() const;

    /**
     * @brief Apply the selection parameters to the rendering context or shader.
     */
    void apply();

    /**
     * @brief Save the current selection state.
     */
    void saveState();

    /**
     * @brief Restore the previously saved selection state.
     */
    void restoreState();

    /**
     * @brief Re-fetch all cached uniform locations from the current shader program.
     * Call this after switching to a different shader program (e.g., PBR) so that
     * apply() writes to the correct uniform locations.
     */
    void refreshForProgram();
};

/**
 * @typedef SelectionManagerPtr
 * @brief Pointer type for SelectionManager singleton.
 */
typedef SelectionManager *SelectionManagerPtr;

}; // namespace ivf
