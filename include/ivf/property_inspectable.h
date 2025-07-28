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
 * @brief Represents different property types that can be exposed for inspection.
 */
using PropertyValue = std::variant<double *, int *, bool *, std::string *, float *, glm::vec3 *, glm::vec4 *,
                                   glm::uint *, glm::uvec3 *, glm::uvec4 *>;

/**
 * @struct Property
 * @brief Property metadata and access structure for property inspection.
 *
 * The Property struct holds metadata and access information for a property that can be
 * inspected or edited at runtime. It supports value pointers, category, range, and read-only flags.
 */
struct Property {
    std::string name;        ///< Name of the property.
    std::string category;    ///< Category for grouping in UI.
    PropertyValue value;     ///< Pointer to the property value.
    double minValue = 0.0;   ///< Minimum value (if range is specified).
    double maxValue = 100.0; ///< Maximum value (if range is specified).
    bool hasRange = false;   ///< True if the property has a range.
    bool readOnly = false;   ///< True if the property is read-only.

    Property(const std::string &n, PropertyValue v, const std::string &cat = "General");
    Property(const std::string &n, PropertyValue v, double min, double max, const std::string &cat = "General");
};

/**
 * @class PropertyInspectable
 * @brief Mixin class to add property inspection capabilities to objects.
 *
 * PropertyInspectable provides a mechanism for objects to expose their properties for
 * inspection and editing, typically for UI editors or debugging tools. Properties can
 * be registered, categorized, and have value ranges or read-only status.
 */
class PropertyInspectable {
private:
    std::vector<Property> m_properties; ///< List of registered properties.

public:
    /**
     * @brief Get all properties for this object (const).
     * @return const std::vector<Property>& List of properties.
     */
    const std::vector<Property> &getProperties() const;

    /**
     * @brief Get all properties, ensuring they are initialized.
     * @return const std::vector<Property>& List of properties.
     */
    const std::vector<Property> &getProperties();

    /**
     * @brief Get properties by category.
     * @param category Category name.
     * @return std::vector<Property> Properties in the given category.
     */
    std::vector<Property> getPropertiesByCategory(const std::string &category) const;

    /**
     * @brief Get all unique property categories.
     * @return std::vector<std::string> List of category names.
     */
    std::vector<std::string> getCategories() const;

    /**
     * @brief Initialize properties for inspection (public wrapper).
     */
    void initializeProperties();

    /**
     * @brief Force re-initialization of properties.
     */
    void refreshProperties();

    /**
     * @brief Notify that a property has changed (public wrapper).
     * @param propertyName Name of the property that changed.
     */
    void notifyPropertyChanged(const std::string &propertyName);

protected:
    /**
     * @brief Register a double property for inspection.
     */
    void addProperty(const std::string &name, double *value, const std::string &category = "General");
    void addProperty(const std::string &name, double *value, double min, double max,
                     const std::string &category = "General");

    /**
     * @brief Register an int property for inspection.
     */
    void addProperty(const std::string &name, int *value, const std::string &category = "General");
    void addProperty(const std::string &name, int *value, double min, double max,
                     const std::string &category = "General");

    /**
     * @brief Register a bool property for inspection.
     */
    void addProperty(const std::string &name, bool *value, const std::string &category = "General");

    /**
     * @brief Register a string property for inspection.
     */
    void addProperty(const std::string &name, std::string *value, const std::string &category = "General");

    /**
     * @brief Register a float property for inspection.
     */
    void addProperty(const std::string &name, float *value, const std::string &category = "General");
    void addProperty(const std::string &name, float *value, double min, double max,
                     const std::string &category = "General");

    /**
     * @brief Register a glm::vec3 property for inspection.
     */
    void addProperty(const std::string &name, glm::vec3 *value, const std::string &category = "General");
    void addPropertyWithRange(const std::string &name, glm::vec3 *value, double minVal, double maxVal,
                              const std::string &category = "General");

    /**
     * @brief Register a glm::vec4 property for inspection.
     */
    void addProperty(const std::string &name, glm::vec4 *value, const std::string &category = "General");
    void addPropertyWithRange(const std::string &name, glm::vec4 *value, double minVal, double maxVal,
                              const std::string &category = "General");

    /**
     * @brief Register a glm::uint property for inspection.
     */
    void addProperty(const std::string &name, glm::uint *value, const std::string &category = "General");
    void addProperty(const std::string &name, glm::uint *value, double min, double max,
                     const std::string &category = "General");

    /**
     * @brief Register a glm::uvec3 property for inspection.
     */
    void addProperty(const std::string &name, glm::uvec3 *value, const std::string &category = "General");
    void addPropertyWithRange(const std::string &name, glm::uvec3 *value, double minVal, double maxVal,
                              const std::string &category = "General");

    /**
     * @brief Register a glm::uvec4 property for inspection.
     */
    void addProperty(const std::string &name, glm::uvec4 *value, const std::string &category = "General");
    void addPropertyWithRange(const std::string &name, glm::uvec4 *value, double minVal, double maxVal,
                              const std::string &category = "General");

    /**
     * @brief Add a read-only property for inspection.
     * @param name Property name.
     * @param value Property value pointer.
     * @param category Category name.
     */
    void addReadOnlyProperty(const std::string &name, PropertyValue value, const std::string &category = "General");

    /**
     * @brief Called when properties are first requested - override to register properties.
     */
    virtual void setupProperties();

    /**
     * @brief Called when a property value changes - override to handle updates.
     * @param propertyName Name of the property that changed.
     */
    virtual void onPropertyChanged(const std::string &propertyName);
};

/**
 * @class PropertyEditor
 * @brief Helper class for UI integration and property editing.
 *
 * PropertyEditor provides static utility functions for converting property values to and from
 * strings, determining property types, and editing individual vector components.
 */
class PropertyEditor {
public:
    /**
     * @brief Get string representation of a property value.
     * @param prop Property to convert.
     * @return std::string String representation.
     */
    static std::string getValueAsString(const Property &prop);

    /**
     * @brief Set property value from a string.
     * @param prop Property to set.
     * @param value String value to parse.
     * @return bool True if successful.
     */
    static bool setValueFromString(const Property &prop, const std::string &value);

    /**
     * @brief Get the property type as a string.
     * @param prop Property to query.
     * @return std::string Type name.
     */
    static std::string getPropertyType(const Property &prop);

    /**
     * @brief Get the name of a vector component.
     * @param prop Property to query.
     * @param component Component index.
     * @return std::string Component name.
     */
    static std::string getComponentName(const Property &prop, int component);

    /**
     * @brief Get the value of a vector component as a float.
     * @param prop Property to query.
     * @param component Component index.
     * @return float Component value.
     */
    static float getComponentValue(const Property &prop, int component);

    /**
     * @brief Set the value of a vector component.
     * @param prop Property to modify.
     * @param component Component index.
     * @param value New value.
     * @return bool True if successful.
     */
    static bool setComponentValue(const Property &prop, int component, float value);

private:
    /**
     * @brief Parse a glm::vec3 from a string.
     * @param str String to parse.
     * @param vec Output vector.
     * @return bool True if successful.
     */
    static bool parseVec3(const std::string &str, glm::vec3 &vec);

    /**
     * @brief Parse a glm::vec4 from a string.
     * @param str String to parse.
     * @param vec Output vector.
     * @return bool True if successful.
     */
    static bool parseVec4(const std::string &str, glm::vec4 &vec);

    /**
     * @brief Parse a glm::uvec3 from a string.
     * @param str String to parse.
     * @param vec Output vector.
     * @return bool True if successful.
     */
    static bool parseUvec3(const std::string &str, glm::uvec3 &vec);

    /**
     * @brief Parse a glm::uvec4 from a string.
     * @param str String to parse.
     * @param vec Output vector.
     * @return bool True if successful.
     */
    static bool parseUvec4(const std::string &str, glm::uvec4 &vec);
};

} // namespace ivf
