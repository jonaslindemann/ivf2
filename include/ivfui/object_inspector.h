// object_inspector.h
#pragma once

#include <ivfui/ui_window.h>
#include <ivf/property_inspectable.h>
#include <memory>
#include <vector>
#include <string>

namespace ivfui {

class ObjectInspector : public UiWindow {
private:
    std::shared_ptr<ivf::PropertyInspectable> m_currentObject;
    std::string m_objectName;
    std::vector<std::string> m_expandedCategories;

    // UI state
    bool m_showAdvanced;
    float m_dragSpeed;

public:
    ObjectInspector();
    virtual ~ObjectInspector();

    static std::shared_ptr<ObjectInspector> create();

    /**
     * Set the object to inspect
     */
    void setObject(std::shared_ptr<ivf::PropertyInspectable> object, const std::string &objectName = "Object");

    /**
     * Clear the current object
     */
    void clearObject();

    /**
     * Check if an object is currently being inspected
     */
    bool hasObject() const;

    /**
     * Get the current object being inspected
     */
    std::shared_ptr<ivf::PropertyInspectable> getCurrentObject() const;

protected:
    virtual void doDraw() override;
    virtual void doUpdate() override;

private:
    /**
     * Draw property controls for a single property
     */
    void drawProperty(const ivf::Property &prop);

    /**
     * Draw vector property with component controls
     */
    void drawVectorProperty(const ivf::Property &prop);

    /**
     * Draw scalar property control
     */
    void drawScalarProperty(const ivf::Property &prop);

    /**
     * Draw boolean property control
     */
    void drawBooleanProperty(const ivf::Property &prop);

    /**
     * Draw string property control
     */
    void drawStringProperty(const ivf::Property &prop);

    /**
     * Check if a category is expanded
     */
    bool isCategoryExpanded(const std::string &category) const;

    /**
     * Set category expansion state
     */
    void setCategoryExpanded(const std::string &category, bool expanded);

    /**
     * Notify object of property change
     */
    void notifyPropertyChanged(const std::string &propertyName);

    /**
     * Generate unique ImGui ID for property
     */
    std::string getPropertyId(const ivf::Property &prop) const;
};

typedef std::shared_ptr<ObjectInspector> ObjectInspectorPtr;

} // namespace ivfui
