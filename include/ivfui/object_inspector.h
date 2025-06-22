// object_inspector.h
#pragma once

/**
 * @file object_inspector.h
 * @brief Declares the ObjectInspector class for property inspection UI in the ivfui library.
 */

#include <ivfui/ui_window.h>
#include <ivf/property_inspectable.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace ivfui {

/**
 * @class ObjectInspector
 * @brief UI window for inspecting and editing properties of PropertyInspectable objects.
 *
 * The ObjectInspector class provides a user interface for viewing and editing the properties
 * of objects that implement the PropertyInspectable interface. It supports property grouping,
 * advanced property display, and change notification for real-time editing and debugging.
 */
class ObjectInspector : public UiWindow {
private:
    std::shared_ptr<ivf::PropertyInspectable> m_currentObject;          ///< Currently inspected object.
    std::string m_objectName;                                           ///< Name of the inspected object.
    std::vector<std::string> m_expandedCategories;                      ///< Expanded property categories in the UI.
    std::function<void(const std::string &)> m_propertyChangedCallback; ///< Callback for property changes.

    // UI state
    bool m_showAdvanced; ///< Show advanced properties flag.
    float m_dragSpeed;   ///< Drag speed for numeric controls.

public:
    /**
     * @brief Construct an ObjectInspector with the given window name.
     * @param name Window name.
     */
    ObjectInspector(const std::string &name);

    /**
     * @brief Destructor.
     */
    virtual ~ObjectInspector();

    void setPropertyChangedCallback(const std::function<void(const std::string &)> &callback)
    {
        m_propertyChangedCallback = callback;
    }

    /**
     * @brief Factory method to create a shared pointer to an ObjectInspector instance.
     * @param name Window name.
     * @return std::shared_ptr<ObjectInspector> New ObjectInspector instance.
     */
    static std::shared_ptr<ObjectInspector> create(const std::string &name);

    /**
     * @brief Set the object to inspect.
     * @param object Shared pointer to the PropertyInspectable object.
     * @param objectName Name to display for the object (default "Object").
     */
    void setObject(std::shared_ptr<ivf::PropertyInspectable> object, const std::string &objectName = "Object");

    /**
     * @brief Clear the current object being inspected.
     */
    void clearObject();

    /**
     * @brief Check if an object is currently being inspected.
     * @return bool True if an object is set.
     */
    bool hasObject() const;

    /**
     * @brief Get the current object being inspected.
     * @return std::shared_ptr<ivf::PropertyInspectable> Inspected object.
     */
    std::shared_ptr<ivf::PropertyInspectable> getCurrentObject() const;

protected:
    /**
     * @brief Draw the object inspector UI (called per frame).
     */
    virtual void doDraw() override;

    /**
     * @brief Update the object inspector state (called per frame).
     */
    virtual void doUpdate() override;

private:
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
     * @brief Notify the inspected object that a property has changed.
     * @param propertyName Name of the changed property.
     */
    void notifyPropertyChanged(const std::string &propertyName);

    /**
     * @brief Generate a unique ImGui ID for a property.
     * @param prop Property to generate ID for.
     * @return std::string Unique ImGui ID.
     */
    std::string getPropertyId(const ivf::Property &prop) const;
};

/**
 * @typedef ObjectInspectorPtr
 * @brief Shared pointer type for ObjectInspector.
 */
typedef std::shared_ptr<ObjectInspector> ObjectInspectorPtr;

} // namespace ivfui
