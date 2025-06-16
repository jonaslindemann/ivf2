#pragma once

/**
 * @file point_light_window.h
 * @brief Declares the PointLightWindow class for point light control UI in the ivfui library.
 */

#include <ivfui/ui_window.h>
#include <ivf/point_light.h>

namespace ivfui {

/**
 * @class PointLightWindow
 * @brief UI window for interactive control of a point light's properties.
 *
 * The PointLightWindow class provides a user interface for viewing and editing
 * the properties of a point light, including attenuation factors, position, diffuse color,
 * specular color, ambient color, and enabled state. It tracks changes for UI-driven updates
 * and supports property inspection and synchronization with the underlying light object.
 */
class PointLightWindow : public ivfui::UiWindow {
private:
    ivf::PointLightPtr m_pointLight; ///< Associated point light.
    float m_constAttenuation;        ///< Constant attenuation factor.
    float m_linearAttenuation;       ///< Linear attenuation factor.
    float m_quadraticAttenuation;    ///< Quadratic attenuation factor.
    glm::vec3 m_position;            ///< Current light position.
    glm::vec3 m_diffuseColor;        ///< Current diffuse color.
    glm::vec3 m_specularColor;       ///< Current specular color.
    glm::vec3 m_ambientColor;        ///< Current ambient color.
    bool m_enabled;                  ///< Current enabled state.

    float m_constAttenuationPrev;     ///< Previous constant attenuation (for change tracking).
    float m_linearAttenuationPrev;    ///< Previous linear attenuation.
    float m_quadraticAttenuationPrev; ///< Previous quadratic attenuation.
    glm::vec3 m_positionPrev;         ///< Previous position.
    glm::vec3 m_diffuseColorPrev;     ///< Previous diffuse color.
    glm::vec3 m_specularColorPrev;    ///< Previous specular color.
    glm::vec3 m_ambientColorPrev;     ///< Previous ambient color.
    bool m_enabledPrev;               ///< Previous enabled state.

    bool m_isDirty; ///< True if light parameters have been modified via the UI.

public:
    /**
     * @brief Construct a PointLightWindow for the given light and caption.
     * @param pointLight Shared pointer to the point light.
     * @param caption Window caption.
     */
    PointLightWindow(ivf::PointLightPtr pointLight, std::string caption);

    /**
     * @brief Factory method to create a shared pointer to a PointLightWindow instance.
     * @param pointLight Shared pointer to the point light.
     * @param caption Window caption.
     * @return std::shared_ptr<PointLightWindow> New PointLightWindow instance.
     */
    static std::shared_ptr<PointLightWindow> create(ivf::PointLightPtr pointLight, std::string caption);

    /**
     * @brief Draw the point light window UI (called per frame).
     */
    void doDraw() override;

    /**
     * @brief Check if the light parameters have been modified via the UI.
     * @return bool True if dirty, false otherwise.
     */
    bool isDirty();
};

/**
 * @typedef PointLightWindowPtr
 * @brief Shared pointer type for PointLightWindow.
 */
typedef std::shared_ptr<PointLightWindow> PointLightWindowPtr;

} // namespace ivfui
