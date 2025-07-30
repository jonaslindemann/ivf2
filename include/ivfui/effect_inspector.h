#pragma once

/**
 * @file effect_inspector.h
 * @brief Declares the EffectInspector class for managing and inspecting post-processing effects in the ivfui library.
 */

#include <ivfui/ui_window.h>
#include <ivf/effect.h>
#include <ivf/property_inspectable.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace ivfui {

/**
 * @class EffectListProvider
 * @brief Interface for providing access to post-processing effects and their management operations.
 *
 * This interface decouples the EffectInspector from specific implementations like GLFWSceneWindow,
 * allowing it to work with any class that can provide access to a list of effects.
 */
class EffectListProvider {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~EffectListProvider() = default;

    /**
     * @brief Get the number of effects in the list.
     * @return int Number of effects.
     */
    virtual int getEffectCount() const = 0;

    /**
     * @brief Get an effect by index.
     * @param index Effect index.
     * @return std::shared_ptr<ivf::Effect> Effect at the given index, or nullptr if invalid.
     */
    virtual std::shared_ptr<ivf::Effect> getEffect(int index) const = 0;

    /**
     * @brief Enable a specific effect by index.
     * @param index Effect index.
     */
    virtual void enableEffect(int index) = 0;

    /**
     * @brief Disable a specific effect by index.
     * @param index Effect index.
     */
    virtual void disableEffect(int index) = 0;

    /**
     * @brief Check if a specific effect is enabled.
     * @param index Effect index.
     * @return bool True if enabled, false otherwise.
     */
    virtual bool isEffectEnabled(int index) const = 0;

    /**
     * @brief Disable all effects.
     */
    virtual void disableAllEffects() = 0;

    /**
     * @brief Remove all effects.
     */
    virtual void clearEffects() = 0;

    /**
     * @brief Reorder an effect from one index to another (optional operation).
     * @param fromIndex Source index.
     * @param toIndex Target index.
     * @return bool True if reordering is supported and was successful.
     */
    virtual bool reorderEffect(int fromIndex, int toIndex)
    {
        return false;
    }
};

/**
 * @class EffectInspector
 * @brief ImGui-based post-processing effects inspector and management window.
 *
 * The EffectInspector class provides a user interface for viewing, managing, and
 * editing post-processing effects. It works with any EffectListProvider to access
 * and manipulate effects, making it independent of specific window implementations.
 */
class EffectInspector : public UiWindow {
private:
    EffectListProvider *m_effectProvider{nullptr};                        ///< Provider for effect list access.
    std::shared_ptr<ivf::Effect> m_selectedEffect{nullptr};               ///< Currently selected effect.
    std::function<void(std::shared_ptr<ivf::Effect>)> m_onEffectSelected; ///< Callback for effect selection.

    // UI state
    bool m_showProperties{true};          ///< Whether to show the properties panel.
    bool m_showAdvancedProperties{false}; ///< Whether to show advanced properties in the properties panel.
    bool m_showOptions{false};            ///< Whether to show inspector options.
    float m_dragSpeed{0.1f};              ///< Drag speed for numeric controls.
    float m_splitterPosition{0.5f};       ///< Position of the splitter between list and properties.
    int m_draggedEffectIndex{-1};         ///< Index of effect being dragged for reordering.

public:
    /**
     * @brief Construct an EffectInspector with the given name.
     * @param name Window name.
     */
    EffectInspector(const std::string &name);

    /**
     * @brief Construct an EffectInspector with the given name and effect provider.
     * @param name Window name.
     * @param effectProvider Pointer to the EffectListProvider.
     */
    EffectInspector(const std::string &name, EffectListProvider *effectProvider);

    /**
     * @brief Virtual destructor.
     */
    virtual ~EffectInspector();

    /**
     * @brief Factory method to create a shared pointer to an EffectInspector instance.
     * @param name Window name.
     * @return std::shared_ptr<EffectInspector> New EffectInspector instance.
     */
    static std::shared_ptr<EffectInspector> create(const std::string &name);

    /**
     * @brief Factory method to create a shared pointer to an EffectInspector instance with effect provider.
     * @param name Window name.
     * @param effectProvider Pointer to the EffectListProvider.
     * @return std::shared_ptr<EffectInspector> New EffectInspector instance.
     */
    static std::shared_ptr<EffectInspector> create(const std::string &name, EffectListProvider *effectProvider);

    /**
     * @brief Set the effect provider for accessing effects.
     * @param effectProvider Pointer to the EffectListProvider.
     */
    void setEffectProvider(EffectListProvider *effectProvider);

    /**
     * @brief Get the current effect provider.
     * @return EffectListProvider* Pointer to the effect provider.
     */
    EffectListProvider *effectProvider() const;

    /**
     * @brief Set the currently selected effect.
     * @param effect Effect to select.
     */
    void setSelectedEffect(std::shared_ptr<ivf::Effect> effect);

    /**
     * @brief Get the currently selected effect.
     * @return std::shared_ptr<ivf::Effect> Currently selected effect.
     */
    std::shared_ptr<ivf::Effect> selectedEffect() const;

    /**
     * @brief Set callback function for when an effect is selected.
     * @param callback Function to call when effect selection changes.
     */
    void setOnEffectSelected(std::function<void(std::shared_ptr<ivf::Effect>)> callback);

    /**
     * @brief Set whether to show the properties panel.
     * @param show True to show properties panel, false to hide it.
     */
    void setShowProperties(bool show);

    /**
     * @brief Check if the properties panel is shown.
     * @return bool True if properties panel is shown.
     */
    bool showProperties() const;

    /**
     * @brief Set whether to show advanced properties in the properties panel.
     * @param show True to show advanced properties, false to hide them.
     */
    void setShowAdvancedProperties(bool show);

    /**
     * @brief Check if advanced properties are shown.
     * @return bool True if advanced properties are shown.
     */
    bool showAdvancedProperties() const;

    /**
     * @brief Set whether to show inspector options.
     * @param show True to show options, false to hide them.
     */
    void setShowOptions(bool show);

    /**
     * @brief Check if inspector options are shown.
     * @return bool True if options are shown.
     */
    bool showOptions() const;

protected:
    /**
     * @brief Draw the effect inspector window contents.
     */
    virtual void doDraw() override;

    /**
     * @brief Get window flags for ImGui.
     */
    virtual ImGuiWindowFlags doWindowFlags() const override;

private:
    /**
     * @brief Draw the effects list.
     */
    void drawEffectsList();

    /**
     * @brief Draw a single effect item in the list.
     * @param effect Effect to draw.
     * @param index Index of the effect in the list.
     * @return bool True if the effect was selected.
     */
    bool drawEffectItem(std::shared_ptr<ivf::Effect> effect, int index);

    /**
     * @brief Get a display name for an effect.
     * @param effect Effect to get name for.
     * @param index Index of the effect in the list.
     * @return std::string Display name for the effect.
     */
    std::string getEffectDisplayName(std::shared_ptr<ivf::Effect> effect, int index) const;

    /**
     * @brief Draw the inspector options/settings.
     */
    void drawInspectorOptions();

    /**
     * @brief Draw the properties panel for the selected effect.
     */
    void drawPropertiesPanel();

    /**
     * @brief Draw property controls for a single property.
     * @param prop Property to draw.
     */
    void drawProperty(const ivf::Property &prop);

    /**
     * @brief Draw vector property with component controls.
     * @param prop Property to draw.
     */
    void drawVectorProperty(const ivf::Property &prop);

    /**
     * @brief Draw scalar property control.
     * @param prop Property to draw.
     */
    void drawScalarProperty(const ivf::Property &prop);

    /**
     * @brief Draw boolean property control.
     * @param prop Property to draw.
     */
    void drawBooleanProperty(const ivf::Property &prop);

    /**
     * @brief Draw string property control.
     * @param prop Property to draw.
     */
    void drawStringProperty(const ivf::Property &prop);

    /**
     * @brief Check if a property category is expanded in the UI.
     * @param category Category name.
     * @return bool True if expanded.
     */
    bool isCategoryExpanded(const std::string &category) const;

    /**
     * @brief Set the expansion state of a property category.
     * @param category Category name.
     * @param expanded True to expand, false to collapse.
     */
    void setCategoryExpanded(const std::string &category, bool expanded);

    /**
     * @brief Generate a unique ImGui ID for a property.
     * @param prop Property to generate ID for.
     * @return std::string Unique ImGui ID.
     */
    std::string getPropertyId(const ivf::Property &prop) const;

    /**
     * @brief Get the number of effects from the provider.
     * @return int Number of effects.
     */
    int getEffectCount() const;

    /**
     * @brief Get an effect by index from the provider.
     * @param index Effect index.
     * @return std::shared_ptr<ivf::Effect> Effect at the given index.
     */
    std::shared_ptr<ivf::Effect> getEffect(int index) const;
};

/**
 * @typedef EffectInspectorPtr
 * @brief Shared pointer type for EffectInspector.
 */
typedef std::shared_ptr<EffectInspector> EffectInspectorPtr;

} // namespace ivfui
