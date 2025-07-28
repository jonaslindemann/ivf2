#include <ivf/property_inspectable.h>
#include <sstream>
#include <algorithm>

namespace ivf {

// Property struct implementations
Property::Property(const std::string &n, PropertyValue v, const std::string &cat) : name(n), value(v), category(cat)
{}

Property::Property(const std::string &n, PropertyValue v, double min, double max, const std::string &cat)
    : name(n), value(v), minValue(min), maxValue(max), hasRange(true), category(cat)
{}

// PropertyInspectable implementations
const std::vector<Property> &PropertyInspectable::getProperties() const
{
    return m_properties;
}

const std::vector<Property> &PropertyInspectable::getProperties()
{
    if (m_properties.empty())
    {
        setupProperties();
    }
    return m_properties;
}

std::vector<Property> PropertyInspectable::getPropertiesByCategory(const std::string &category) const
{
    std::vector<Property> result;
    for (const auto &prop : m_properties)
    {
        if (prop.category == category)
        {
            result.push_back(prop);
        }
    }
    return result;
}

std::vector<std::string> PropertyInspectable::getCategories() const
{
    std::vector<std::string> categories;
    for (const auto &prop : m_properties)
    {
        if (std::find(categories.begin(), categories.end(), prop.category) == categories.end())
        {
            categories.push_back(prop.category);
        }
    }
    return categories;
}

void PropertyInspectable::addProperty(const std::string &name, double *value, const std::string &category)
{
    m_properties.emplace_back(name, value, category);
}

void PropertyInspectable::addProperty(const std::string &name, double *value, double min, double max,
                                      const std::string &category)
{
    m_properties.emplace_back(name, value, min, max, category);
}

void PropertyInspectable::addProperty(const std::string &name, int *value, const std::string &category)
{
    m_properties.emplace_back(name, value, category);
}

void PropertyInspectable::addProperty(const std::string &name, int *value, double min, double max,
                                      const std::string &category)
{
    Property prop(name, value, min, max, category);
    m_properties.push_back(prop);
}

void PropertyInspectable::addProperty(const std::string &name, bool *value, const std::string &category)
{
    m_properties.emplace_back(name, value, category);
}

void PropertyInspectable::addProperty(const std::string &name, std::string *value, const std::string &category)
{
    m_properties.emplace_back(name, value, category);
}

void PropertyInspectable::addProperty(const std::string &name, float *value, const std::string &category)
{
    m_properties.emplace_back(name, value, category);
}

void PropertyInspectable::addProperty(const std::string &name, float *value, double min, double max,
                                      const std::string &category)
{
    Property prop(name, value, min, max, category);
    m_properties.push_back(prop);
}

void PropertyInspectable::addProperty(const std::string &name, glm::vec3 *value, const std::string &category)
{
    m_properties.emplace_back(name, value, category);
}

void PropertyInspectable::addPropertyWithRange(const std::string &name, glm::vec3 *value, double minVal, double maxVal,
                                               const std::string &category)
{
    Property prop(name, value, minVal, maxVal, category);
    m_properties.push_back(prop);
}

void PropertyInspectable::addProperty(const std::string &name, glm::vec4 *value, const std::string &category)
{
    m_properties.emplace_back(name, value, category);
}

void PropertyInspectable::addPropertyWithRange(const std::string &name, glm::vec4 *value, double minVal, double maxVal,
                                               const std::string &category)
{
    Property prop(name, value, minVal, maxVal, category);
    m_properties.push_back(prop);
}

void PropertyInspectable::addProperty(const std::string &name, glm::uvec3 *value, const std::string &category)
{
    m_properties.emplace_back(name, value, category);
}

void PropertyInspectable::addPropertyWithRange(const std::string &name, glm::uvec3 *value, double minVal, double maxVal,
                                               const std::string &category)
{
    Property prop(name, value, minVal, maxVal, category);
    m_properties.push_back(prop);
}

void PropertyInspectable::addReadOnlyProperty(const std::string &name, PropertyValue value, const std::string &category)
{
    Property prop(name, value, category);
    prop.readOnly = true;
    m_properties.push_back(prop);
}

void PropertyInspectable::setupProperties()
{
    // Default implementation does nothing
    // Override in derived classes to register properties
}

void PropertyInspectable::onPropertyChanged(const std::string &propertyName)
{
    // Default implementation does nothing
    // Override in derived classes to handle property changes
}

void PropertyInspectable::initializeProperties()
{
    if (m_properties.empty())
    {
        setupProperties();
    }
}

void PropertyInspectable::refreshProperties()
{
    m_properties.clear();
    setupProperties();
}

void PropertyInspectable::notifyPropertyChanged(const std::string &propertyName)
{
    onPropertyChanged(propertyName);
}

// PropertyEditor implementations
std::string PropertyEditor::getValueAsString(const Property &prop)
{
    return std::visit(
        [](auto &&arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, double *>)
            {
                return std::to_string(*arg);
            }
            else if constexpr (std::is_same_v<T, int *>)
            {
                return std::to_string(*arg);
            }
            else if constexpr (std::is_same_v<T, glm::uint *>)
            {
                return std::to_string(*arg);
            }
            else if constexpr (std::is_same_v<T, bool *>)
            {
                return *arg ? "true" : "false";
            }
            else if constexpr (std::is_same_v<T, std::string *>)
            {
                return *arg;
            }
            else if constexpr (std::is_same_v<T, float *>)
            {
                return std::to_string(*arg);
            }
            else if constexpr (std::is_same_v<T, glm::vec3 *>)
            {
                return "(" + std::to_string((*arg)[0]) + ", " + std::to_string((*arg)[1]) + ", " +
                       std::to_string((*arg)[2]) + ")";
            }
            else if constexpr (std::is_same_v<T, glm::vec4 *>)
            {
                return "(" + std::to_string((*arg)[0]) + ", " + std::to_string((*arg)[1]) + ", " +
                       std::to_string((*arg)[2]) + ", " + std::to_string((*arg)[3]) + ")";
            }
            return "";
        },
        prop.value);
}

bool PropertyEditor::setValueFromString(const Property &prop, const std::string &value)
{
    try
    {
        return std::visit(
            [&value](auto &&arg) -> bool {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, double *>)
                {
                    *arg = std::stod(value);
                    return true;
                }
                else if constexpr (std::is_same_v<T, int *>)
                {
                    *arg = std::stoi(value);
                    return true;
                }
                else if constexpr (std::is_same_v<T, glm::uint *>)
                {
                    *arg = std::stoi(value);
                    return true;
                }
                else if constexpr (std::is_same_v<T, bool *>)
                {
                    *arg = (value == "true" || value == "1");
                    return true;
                }
                else if constexpr (std::is_same_v<T, std::string *>)
                {
                    *arg = value;
                    return true;
                }
                else if constexpr (std::is_same_v<T, float *>)
                {
                    *arg = std::stof(value);
                    return true;
                }
                else if constexpr (std::is_same_v<T, glm::vec3 *>)
                {
                    return parseVec3(value, *arg);
                }
                else if constexpr (std::is_same_v<T, glm::vec4 *>)
                {
                    return parseVec4(value, *arg);
                }
                return false;
            },
            prop.value);
    } catch (...)
    {
        return false;
    }
}

std::string PropertyEditor::getPropertyType(const Property &prop)
{
    return std::visit(
        [](auto &&arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, double *>)
            {
                return "double";
            }
            else if constexpr (std::is_same_v<T, int *>)
            {
                return "int";
            }
            else if constexpr (std::is_same_v<T, glm::uint *>)
            {
                return "uint";
            }
            else if constexpr (std::is_same_v<T, bool *>)
            {
                return "bool";
            }
            else if constexpr (std::is_same_v<T, std::string *>)
            {
                return "string";
            }
            else if constexpr (std::is_same_v<T, float *>)
            {
                return "float";
            }
            else if constexpr (std::is_same_v<T, glm::vec3 *>)
            {
                return "vec3";
            }
            else if constexpr (std::is_same_v<T, glm::vec4 *>)
            {
                return "vec4";
            }
            else if constexpr (std::is_same_v<T, glm::uvec3 *>)
            {
                return "uvec3";
            }
            else if constexpr (std::is_same_v<T, glm::uvec4 *>)
            {
                return "uvec4";
            }
            return "unknown";
        },
        prop.value);
}

std::string PropertyEditor::getComponentName(const Property &prop, int component)
{
    std::string type = getPropertyType(prop);
    if (type == "vec3" || type == "vec4")
    {
        const char *components[] = {"X", "Y", "Z", "W"};
        if (component >= 0 && component < 4)
        {
            return prop.name + " " + components[component];
        }
    }
    return prop.name;
}

float PropertyEditor::getComponentValue(const Property &prop, int component)
{
    return std::visit(
        [component](auto &&arg) -> float {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, glm::vec3 *>)
            {
                if (component >= 0 && component < 3)
                {
                    return (*arg)[component];
                }
            }
            else if constexpr (std::is_same_v<T, glm::vec4 *>)
            {
                if (component >= 0 && component < 4)
                {
                    return (*arg)[component];
                }
            }
            return 0.0f;
        },
        prop.value);
}

bool PropertyEditor::setComponentValue(const Property &prop, int component, float value)
{
    return std::visit(
        [component, value](auto &&arg) -> bool {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, glm::vec3 *>)
            {
                if (component >= 0 && component < 3)
                {
                    (*arg)[component] = value;
                    return true;
                }
            }
            else if constexpr (std::is_same_v<T, glm::vec4 *>)
            {
                if (component >= 0 && component < 4)
                {
                    (*arg)[component] = value;
                    return true;
                }
            }
            return false;
        },
        prop.value);
}

bool PropertyEditor::parseVec3(const std::string &str, glm::vec3 &vec)
{
    std::string cleaned = str;

    // Remove parentheses and replace commas with spaces
    std::replace(cleaned.begin(), cleaned.end(), '(', ' ');
    std::replace(cleaned.begin(), cleaned.end(), ')', ' ');
    std::replace(cleaned.begin(), cleaned.end(), ',', ' ');

    std::istringstream iss(cleaned);
    float x, y, z;

    if (iss >> x >> y >> z)
    {
        vec = glm::vec3(x, y, z);
        return true;
    }
    return false;
}

bool PropertyEditor::parseVec4(const std::string &str, glm::vec4 &vec)
{
    std::string cleaned = str;

    // Remove parentheses and replace commas with spaces
    std::replace(cleaned.begin(), cleaned.end(), '(', ' ');
    std::replace(cleaned.begin(), cleaned.end(), ')', ' ');
    std::replace(cleaned.begin(), cleaned.end(), ',', ' ');

    std::istringstream iss(cleaned);
    float x, y, z, w;

    if (iss >> x >> y >> z >> w)
    {
        vec = glm::vec4(x, y, z, w);
        return true;
    }
    return false;
}

} // namespace ivf
