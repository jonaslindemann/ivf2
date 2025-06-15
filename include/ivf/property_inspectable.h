#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <variant>
#include <typeinfo>
#include <glm/glm.hpp>

namespace ivf {

// Forward declaration
class Base;

/**
 * Represents different property types that can be exposed
 */
using PropertyValue = std::variant<double *, int *, bool *, std::string *, float *, glm::vec3 *, glm::vec4 *>;

/**
 * Property metadata and access structure
 */
struct Property {
    std::string name;
    std::string category;
    PropertyValue value;
    double minValue = 0.0;
    double maxValue = 100.0;
    bool hasRange = false;
    bool readOnly = false;

    Property(const std::string &n, PropertyValue v, const std::string &cat = "General");
    Property(const std::string &n, PropertyValue v, double min, double max, const std::string &cat = "General");
};

/**
 * Mixin class to add property inspection capabilities
 */
class PropertyInspectable {
private:
    std::vector<Property> m_properties;

public:
    /**
     * Get all properties for this object
     */
    const std::vector<Property> &getProperties() const;

    /**
     * Get all properties, ensuring they are initialized
     */
    const std::vector<Property> &getProperties();

    /**
     * Get properties by category
     */
    std::vector<Property> getPropertiesByCategory(const std::string &category) const;

    /**
     * Get all unique categories
     */
    std::vector<std::string> getCategories() const;

    /**
     * Initialize properties for inspection
     * This is a public wrapper around the protected setupProperties()
     */
    void initializeProperties();

    /**
     * Force re-initialization of properties
     */
    void refreshProperties();

    /**
     * Notify that a property has changed (public wrapper)
     */
    void notifyPropertyChanged(const std::string &propertyName);

protected:
    /**
     * Register properties for inspection
     */
    void addProperty(const std::string &name, double *value, const std::string &category = "General");
    void addProperty(const std::string &name, double *value, double min, double max,
                     const std::string &category = "General");
    void addProperty(const std::string &name, int *value, const std::string &category = "General");
    void addProperty(const std::string &name, int *value, double min, double max,
                     const std::string &category = "General");
    void addProperty(const std::string &name, bool *value, const std::string &category = "General");
    void addProperty(const std::string &name, std::string *value, const std::string &category = "General");
    void addProperty(const std::string &name, float *value, const std::string &category = "General");
    void addProperty(const std::string &name, float *value, double min, double max,
                     const std::string &category = "General");
    void addProperty(const std::string &name, glm::vec3 *value, const std::string &category = "General");
    void addPropertyWithRange(const std::string &name, glm::vec3 *value, double minVal, double maxVal,
                              const std::string &category = "General");
    void addProperty(const std::string &name, glm::vec4 *value, const std::string &category = "General");
    void addPropertyWithRange(const std::string &name, glm::vec4 *value, double minVal, double maxVal,
                              const std::string &category = "General");

    /**
     * Add read-only property
     */
    void addReadOnlyProperty(const std::string &name, PropertyValue value, const std::string &category = "General");

    /**
     * Called when properties are first requested - override to register properties
     */
    virtual void setupProperties();

    /**
     * Called when a property value changes - override to handle updates
     */
    virtual void onPropertyChanged(const std::string &propertyName);
};

/**
 * Helper class for UI integration
 */
class PropertyEditor {
public:
    /**
     * Get string representation of property value
     */
    static std::string getValueAsString(const Property &prop);

    /**
     * Set property value from string
     */
    static bool setValueFromString(const Property &prop, const std::string &value);

    /**
     * Get property type as string
     */
    static std::string getPropertyType(const Property &prop);

    /**
     * Get individual component of vector properties
     */
    static std::string getComponentName(const Property &prop, int component);

    /**
     * Get component value as float
     */
    static float getComponentValue(const Property &prop, int component);

    /**
     * Set component value
     */
    static bool setComponentValue(const Property &prop, int component, float value);

private:
    /**
     * Parse vec3 from string format: "(x, y, z)" or "x y z" or "x,y,z"
     */
    static bool parseVec3(const std::string &str, glm::vec3 &vec);

    /**
     * Parse vec4 from string format: "(x, y, z, w)" or "x y z w" or "x,y,z,w"
     */
    static bool parseVec4(const std::string &str, glm::vec4 &vec);
};

} // namespace ivf
