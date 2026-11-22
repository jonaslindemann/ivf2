#pragma once

#include <string>
#include <string_view>
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
    PropertyInspectable() = default;
    virtual ~PropertyInspectable() = default;

    // Properties hold pointers into this object, so a copy (or a moved-to object, e.g. in a
    // std::vector that reallocates) starts without them and registers its own on first use.
    PropertyInspectable(const PropertyInspectable &)
    {
    }
    PropertyInspectable(PropertyInspectable &&) noexcept
    {
    }
    PropertyInspectable &operator=(const PropertyInspectable &)
    {
        m_properties.clear();
        return *this;
    }
    PropertyInspectable &operator=(PropertyInspectable &&) noexcept
    {
        m_properties.clear();
        return *this;
    }

    /**
     * @brief Get all properties for this object (const).
     * @return const std::vector<Property>& List of properties.
     */
    const std::vector<Property> &properties() const;
    [[deprecated("Use properties() instead")]] const std::vector<Property> &getProperties() const { return properties(); }

    /**
     * @brief Get all properties, ensuring they are initialized.
     * @return const std::vector<Property>& List of properties.
     */
    const std::vector<Property> &properties();
    [[deprecated("Use properties() instead")]] const std::vector<Property> &getProperties() { return properties(); }

    /**
     * @brief Get properties by category.
     * @param category Category name.
     * @return std::vector<Property> Properties in the given category.
     */
    std::vector<Property> propertiesByCategory(const std::string &category) const;
    [[deprecated("Use propertiesByCategory() instead")]] std::vector<Property> getPropertiesByCategory(const std::string &category) const { return propertiesByCategory(category); }

    /**
     * @brief Get all unique property categories.
     * @return std::vector<std::string> List of category names.
     */
    std::vector<std::string> categories() const;
    [[deprecated("Use categories() instead")]] std::vector<std::string> getCategories() const { return categories(); }

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
    void addProperty(std::string_view name, double *value, std::string_view category = "General");
    void addProperty(std::string_view name, double *value, double min, double max,
                     std::string_view category = "General");

    /**
     * @brief Register an int property for inspection.
     */
    void addProperty(std::string_view name, int *value, std::string_view category = "General");
    void addProperty(std::string_view name, int *value, double min, double max,
                     std::string_view category = "General");

    /**
     * @brief Register a bool property for inspection.
     */
    void addProperty(std::string_view name, bool *value, std::string_view category = "General");

    /**
     * @brief Register a string property for inspection.
     */
    void addProperty(std::string_view name, std::string *value, std::string_view category = "General");

    /**
     * @brief Register a float property for inspection.
     */
    void addProperty(std::string_view name, float *value, std::string_view category = "General");
    void addProperty(std::string_view name, float *value, double min, double max,
                     std::string_view category = "General");

    /**
     * @brief Register a glm::vec3 property for inspection.
     */
    void addProperty(std::string_view name, glm::vec3 *value, std::string_view category = "General");
    void addPropertyWithRange(std::string_view name, glm::vec3 *value, double minVal, double maxVal,
                              std::string_view category = "General");

    /**
     * @brief Register a glm::vec4 property for inspection.
     */
    void addProperty(std::string_view name, glm::vec4 *value, std::string_view category = "General");
    void addPropertyWithRange(std::string_view name, glm::vec4 *value, double minVal, double maxVal,
                              std::string_view category = "General");

    /**
     * @brief Register a glm::uint property for inspection.
     */
    void addProperty(std::string_view name, glm::uint *value, std::string_view category = "General");
    void addProperty(std::string_view name, glm::uint *value, double min, double max,
                     std::string_view category = "General");

    /**
     * @brief Register a glm::uvec3 property for inspection.
     */
    void addProperty(std::string_view name, glm::uvec3 *value, std::string_view category = "General");
    void addPropertyWithRange(std::string_view name, glm::uvec3 *value, double minVal, double maxVal,
                              std::string_view category = "General");

    /**
     * @brief Register a glm::uvec4 property for inspection.
     */
    void addProperty(std::string_view name, glm::uvec4 *value, std::string_view category = "General");
    void addPropertyWithRange(std::string_view name, glm::uvec4 *value, double minVal, double maxVal,
                              std::string_view category = "General");

    /**
     * @brief Add a read-only property for inspection.
     * @param name Property name.
     * @param value Property value pointer.
     * @param category Category name.
     */
    void addReadOnlyProperty(std::string_view name, PropertyValue value, std::string_view category = "General");

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
    static std::string valueAsString(const Property &prop);
    [[deprecated("Use valueAsString() instead")]] static std::string getValueAsString(const Property &prop) { return valueAsString(prop); }

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
    static std::string propertyType(const Property &prop);
    [[deprecated("Use propertyType() instead")]] static std::string getPropertyType(const Property &prop) { return propertyType(prop); }

    /**
     * @brief Get the name of a vector component.
     * @param prop Property to query.
     * @param component Component index.
     * @return std::string Component name.
     */
    static std::string componentName(const Property &prop, int component);
    [[deprecated("Use componentName() instead")]] static std::string getComponentName(const Property &prop, int component) { return componentName(prop, component); }

    /**
     * @brief Get the value of a vector component as a float.
     * @param prop Property to query.
     * @param component Component index.
     * @return float Component value.
     */
    static float componentValue(const Property &prop, int component);
    [[deprecated("Use componentValue() instead")]] static float getComponentValue(const Property &prop, int component) { return componentValue(prop, component); }

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
