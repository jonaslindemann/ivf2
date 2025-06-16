#pragma once

/**
 * @file spot_light_window.h
 * @brief Declares the SpotLightWindow class for spot light control UI in the ivfui library.
 */

#include <ivfui/ui_window.h>
#include <ivf/spot_light.h>

namespace ivfui {

/**
 * @class SpotLightWindow
 * @brief UI window for interactive control of a spot light's properties.
 *
 * The SpotLightWindow class provides a user interface for viewing and editing
 * the properties of a spot light, including attenuation factors, cutoff angles,
 * direction, position, diffuse color, specular color, ambient color, and enabled state.
 * It tracks changes for UI-driven updates and supports property inspection and
 * synchronization with the underlying light object.
 */
class SpotLightWindow : public ivfui::UiWindow {
private:
    ivf::SpotLightPtr m_spotLight; ///< Associated spot light.
    float m_constAttenuation;      ///< Constant attenuation factor.
    float m_linearAttenuation;     ///< Linear attenuation factor.
    float m_quadraticAttenuation;  ///< Quadratic attenuation factor.
    float m_cutoff;                ///< Inner cutoff angle (degrees or radians).
    float m_outerCutoff;           ///< Outer cutoff angle (degrees or radians).
    glm::vec3 m_direction;         ///< Current light direction.
    glm::vec3 m_position;          ///< Current light position.
    glm::vec3 m_diffuseColor;      ///< Current diffuse color.
    glm::vec3 m_specularColor;     ///< Current specular color.
    glm::vec3 m_ambientColor;      ///< Current ambient color.
    bool m_enabled;                ///< Current enabled state.

    float m_constAttenuationPrev;     ///< Previous constant attenuation (for change tracking).
    float m_linearAttenuationPrev;    ///< Previous linear attenuation.
    float m_quadraticAttenuationPrev; ///< Previous quadratic attenuation.
    float m_cutoffPrev;               ///< Previous inner cutoff angle.
    float m_outerCutoffPrev;          ///< Previous outer cutoff angle.
    glm::vec3 m_directionPrev;        ///< Previous direction.
    glm::vec3 m_positionPrev;         ///< Previous position.
    glm::vec3 m_diffuseColorPrev;     ///< Previous diffuse color.
    glm::vec3 m_specularColorPrev;    ///< Previous specular color.
    glm::vec3 m_ambientColorPrev;     ///< Previous ambient color.
    bool m_enabledPrev;               ///< Previous enabled state.

    bool m_isDirty; ///< True if light parameters have been modified via the UI.

public:
    /**
     * @brief Construct a SpotLightWindow for the given light and caption.
     * @param spotLight Shared pointer to the spot light.
     * @param caption Window caption.
     */
    SpotLightWindow(ivf::SpotLightPtr spotLight, std::string caption);

    /**
     * @brief Factory method to create a shared pointer to a SpotLightWindow instance.
     * @param spotLight Shared pointer to the spot light.
     * @param caption Window caption.
     * @return std::shared_ptr<SpotLightWindow> New SpotLightWindow instance.
     */
    static std::shared_ptr<SpotLightWindow> create(ivf::SpotLightPtr spotLight, std::string caption);

    /**
     * @brief Draw the spot light window UI (called per frame).
     */
    void doDraw() override;

    /**
     * @brief Check if the light parameters have been modified via the UI.
     * @return bool True if dirty, false otherwise.
     */
    bool isDirty();
};

/**
 * @typedef SpotLightWindowPtr
 * @brief Shared pointer type for SpotLightWindow.
 */
typedef std::shared_ptr<SpotLightWindow> SpotLightWindowPtr;

} // namespace ivfui
