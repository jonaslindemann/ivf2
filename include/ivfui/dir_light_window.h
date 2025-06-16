#pragma once

/**
 * @file dir_light_window.h
 * @brief Declares the DirectionalLightWindow class for directional light control UI in the ivfui library.
 */

#include <ivfui/ui_window.h>
#include <ivf/dir_light.h>

namespace ivfui {

/**
 * @class DirectionalLightWindow
 * @brief UI window for interactive control of a directional light's properties.
 *
 * The DirectionalLightWindow class provides a user interface for viewing and editing
 * the properties of a directional light, including direction, diffuse color, specular color,
 * ambient color, and enabled state. It tracks changes for UI-driven updates and supports
 * property inspection and synchronization with the underlying light object.
 */
class DirectionalLightWindow : public ivfui::UiWindow {
private:
    ivf::DirectionalLightPtr m_dirLight; ///< Associated directional light.
    glm::vec3 m_direction;               ///< Current light direction.
    glm::vec3 m_diffuseColor;            ///< Current diffuse color.
    glm::vec3 m_specularColor;           ///< Current specular color.
    glm::vec3 m_ambientColor;            ///< Current ambient color.
    bool m_enabled;                      ///< Current enabled state.

    glm::vec3 m_directionPrev;     ///< Previous light direction (for change tracking).
    glm::vec3 m_diffuseColorPrev;  ///< Previous diffuse color.
    glm::vec3 m_specularColorPrev; ///< Previous specular color.
    glm::vec3 m_ambientColorPrev;  ///< Previous ambient color.
    bool m_enabledPrev;            ///< Previous enabled state.

    bool m_isDirty; ///< True if light parameters have been modified via the UI.

public:
    /**
     * @brief Construct a DirectionalLightWindow for the given light and caption.
     * @param dirLight Shared pointer to the directional light.
     * @param caption Window caption.
     */
    DirectionalLightWindow(ivf::DirectionalLightPtr dirLight, std::string caption);

    /**
     * @brief Factory method to create a shared pointer to a DirectionalLightWindow instance.
     * @param dirLight Shared pointer to the directional light.
     * @param caption Window caption.
     * @return std::shared_ptr<DirectionalLightWindow> New DirectionalLightWindow instance.
     */
    static std::shared_ptr<DirectionalLightWindow> create(ivf::DirectionalLightPtr dirLight, std::string caption);

    /**
     * @brief Draw the directional light window UI (called per frame).
     */
    void doDraw() override;

    /**
     * @brief Check if the light parameters have been modified via the UI.
     * @return bool True if dirty, false otherwise.
     */
    bool isDirty();
};

/**
 * @typedef DirectionalLightWindowPtr
 * @brief Shared pointer type for DirectionalLightWindow.
 */
typedef std::shared_ptr<DirectionalLightWindow> DirectionalLightWindowPtr;

} // namespace ivfui
