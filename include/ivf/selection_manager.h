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
private:
    SelectionManager();                  ///< Private constructor for singleton pattern.
    static SelectionManager *m_instance; ///< Singleton instance pointer.

    uint32_t m_objectId;      ///< Current object ID for selection.
    uint32_t m_savedObjectId; ///< Saved object ID for state restoration.
    GLint m_objectIdId;       ///< Shader uniform location for object ID.

    bool m_selectionRendering;      ///< Whether selection rendering is active.
    bool m_savedSelectionRendering; ///< Saved selection rendering state.
    GLint m_selectionRenderingId;   ///< Shader uniform location for selection rendering.

public:
    /**
     * @brief Get the singleton instance of the SelectionManager.
     * @return SelectionManager* Pointer to the singleton instance.
     */
    static SelectionManager *instance()
    {
        if (!m_instance)
            m_instance = new SelectionManager();

        return m_instance;
    }

    /**
     * @brief Create the singleton instance of the SelectionManager (if not already created).
     * @return SelectionManager* Pointer to the singleton instance.
     */
    static SelectionManager *create()
    {
        return instance();
    }

    /**
     * @brief Destroy the singleton instance and release all resources.
     */
    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    /**
     * @brief Set the current object ID for selection.
     * @param objectId Object ID value.
     */
    void setObjectId(uint32_t objectId);

    /**
     * @brief Get the current object ID for selection.
     * @return uint32_t Object ID value.
     */
    uint32_t objectId();

    /**
     * @brief Enable or disable selection rendering.
     * @param selectionRendering True to enable, false to disable.
     */
    void setSelectionRendering(bool selectionRendering);

    /**
     * @brief Check if selection rendering is enabled.
     * @return bool True if selection rendering is enabled.
     */
    bool selectionRendering();

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
};

/**
 * @typedef SelectionManagerPtr
 * @brief Pointer type for SelectionManager singleton.
 */
typedef SelectionManager *SelectionManagerPtr;

}; // namespace ivf
