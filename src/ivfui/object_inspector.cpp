#include <ivfui/object_inspector.h>
#include <imgui.h>
#include <algorithm>
#include <sstream>

using namespace ivfui;

ObjectInspector::ObjectInspector(const std::string &name)
    : UiWindow(name), m_currentObject(nullptr), m_showAdvanced(false), m_dragSpeed(0.1f)
{}

ObjectInspector::~ObjectInspector()
{}

std::shared_ptr<ObjectInspector> ObjectInspector::create(const std::string &name)
{
    return std::make_shared<ObjectInspector>(name);
}

void ObjectInspector::setObject(std::shared_ptr<ivf::PropertyInspectable> object, const std::string &objectName)
{
    m_currentObject = object;
    m_objectName = objectName;

    if (m_currentObject)
    {
        // Initialize properties when object is assigned
        m_currentObject->initializeProperties();

        // Auto-expand all categories initially
        auto categories = m_currentObject->getCategories();
        for (const auto &category : categories)
        {
            setCategoryExpanded(category, true);
        }
    }
}

void ObjectInspector::clearObject()
{
    m_currentObject = nullptr;
    m_objectName.clear();
    m_expandedCategories.clear();
}

bool ObjectInspector::hasObject() const
{
    return m_currentObject != nullptr;
}

std::shared_ptr<ivf::PropertyInspectable> ObjectInspector::getCurrentObject() const
{
    return m_currentObject;
}

void ObjectInspector::doDraw()
{
    if (!m_currentObject)
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No object selected");
        ImGui::Separator();
        ImGui::Text("Select an object to inspect its properties");
        return;
    }

    // Object header
    if (m_objectName != "Object")
    {
        ImGui::Text("Object: %s", m_objectName.c_str());
        ImGui::Separator();
    }

    // Settings
    if (ImGui::CollapsingHeader("Inspector Settings"))
    {
        ImGui::Checkbox("Show Advanced", &m_showAdvanced);
        ImGui::SliderFloat("Drag Speed", &m_dragSpeed, 0.01f, 1.0f, "%.2f");

        if (ImGui::Button("Refresh Properties"))
        {
            if (m_currentObject)
            {
                m_currentObject->refreshProperties();
                // Re-expand categories
                auto categories = m_currentObject->getCategories();
                for (const auto &category : categories)
                {
                    setCategoryExpanded(category, true);
                }
            }
        }

        ImGui::Separator();
    }

    // Get properties and categories
    const auto &properties = m_currentObject->getProperties();
    auto categories = m_currentObject->getCategories();

    if (properties.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "No properties to display");
        return;
    }

    // Draw properties by category
    for (const auto &category : categories)
    {
        bool expanded = isCategoryExpanded(category);

        // Category header with expand/collapse
        if (ImGui::CollapsingHeader(category.c_str(), expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0))
        {
            setCategoryExpanded(category, true);

            // Get properties for this category
            auto categoryProps = m_currentObject->getPropertiesByCategory(category);

            // Draw each property in the category
            for (const auto &prop : categoryProps)
            {
                ImGui::PushID(getPropertyId(prop).c_str());
                drawProperty(prop);
                ImGui::PopID();
            }

            ImGui::Spacing();
        }
        else
        {
            setCategoryExpanded(category, false);
        }
    }
}

void ObjectInspector::doUpdate()
{
    // Update logic if needed
}

void ObjectInspector::drawProperty(const ivf::Property &prop)
{
    std::string type = ivf::PropertyEditor::getPropertyType(prop);

    // Skip read-only properties if not showing advanced
    if (prop.readOnly && !m_showAdvanced)
    {
        return;
    }

    // Property label with read-only indicator
    std::string label = prop.name;
    if (prop.readOnly)
    {
        label += " (read-only)";
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", label.c_str());
    }

    // Draw appropriate control based on type
    if (type == "vec3" || type == "vec4")
    {
        drawVectorProperty(prop);
    }
    else if (type == "bool")
    {
        drawBooleanProperty(prop);
    }
    else if (type == "string")
    {
        drawStringProperty(prop);
    }
    else
    {
        drawScalarProperty(prop);
    }

    // Add tooltip with property info
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Type: %s", type.c_str());
        ImGui::Text("Category: %s", prop.category.c_str());
        if (prop.hasRange)
        {
            ImGui::Text("Range: [%.3f - %.3f]", prop.minValue, prop.maxValue);
        }
        if (prop.readOnly)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.7f, 1.0f), "Read-only property");
        }
        ImGui::EndTooltip();
    }
}

void ObjectInspector::drawVectorProperty(const ivf::Property &prop)
{
    std::string type = ivf::PropertyEditor::getPropertyType(prop);
    int componentCount = (type == "vec3") ? 3 : 4;

    // Vector display with components
    ImGui::Text("%s", prop.name.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%s)", type.c_str());

    // Component sliders
    bool changed = false;
    const char *componentLabels[] = {"X", "Y", "Z", "W"};

    for (int i = 0; i < componentCount; ++i)
    {
        float value = ivf::PropertyEditor::getComponentValue(prop, i);
        float oldValue = value;

        std::string componentId = std::string("##") + prop.name + "_" + componentLabels[i];

        ImGui::Text("%s:", componentLabels[i]);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80.0f);

        if (prop.readOnly)
        {
            ImGui::BeginDisabled();
        }

        if (prop.hasRange)
        {
            if (ImGui::SliderFloat(componentId.c_str(), &value, static_cast<float>(prop.minValue),
                                   static_cast<float>(prop.maxValue), "%.3f"))
            {
                if (value != oldValue)
                {
                    ivf::PropertyEditor::setComponentValue(prop, i, value);
                    changed = true;
                }
            }
        }
        else
        {
            if (ImGui::DragFloat(componentId.c_str(), &value, m_dragSpeed, 0.0f, 0.0f, "%.3f"))
            {
                if (value != oldValue)
                {
                    ivf::PropertyEditor::setComponentValue(prop, i, value);
                    changed = true;
                }
            }
        }

        if (prop.readOnly)
        {
            ImGui::EndDisabled();
        }

        if (i < componentCount - 1)
        {
            ImGui::SameLine();
        }
    }

    if (changed)
    {
        notifyPropertyChanged(prop.name);
    }
}

void ObjectInspector::drawScalarProperty(const ivf::Property &prop)
{
    std::string type = ivf::PropertyEditor::getPropertyType(prop);

    if (type == "double" || type == "float")
    {
        float value = std::visit(
            [](auto &&arg) -> float {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, double *>)
                {
                    return static_cast<float>(*arg);
                }
                else if constexpr (std::is_same_v<T, float *>)
                {
                    return *arg;
                }
                return 0.0f;
            },
            prop.value);

        float oldValue = value;

        if (prop.readOnly)
        {
            ImGui::BeginDisabled();
        }

        if (prop.hasRange)
        {
            if (ImGui::SliderFloat(prop.name.c_str(), &value, static_cast<float>(prop.minValue),
                                   static_cast<float>(prop.maxValue), "%.3f"))
            {
                // Update the actual property
                std::visit(
                    [value](auto &&arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, double *>)
                        {
                            *arg = static_cast<double>(value);
                        }
                        else if constexpr (std::is_same_v<T, float *>)
                        {
                            *arg = value;
                        }
                    },
                    prop.value);

                notifyPropertyChanged(prop.name);
            }
        }
        else
        {
            if (ImGui::DragFloat(prop.name.c_str(), &value, m_dragSpeed, 0.0f, 0.0f, "%.3f"))
            {
                // Update the actual property
                std::visit(
                    [value](auto &&arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, double *>)
                        {
                            *arg = static_cast<double>(value);
                        }
                        else if constexpr (std::is_same_v<T, float *>)
                        {
                            *arg = value;
                        }
                    },
                    prop.value);

                notifyPropertyChanged(prop.name);
            }
        }

        if (prop.readOnly)
        {
            ImGui::EndDisabled();
        }
    }
    else if (type == "int")
    {
        int *intPtr = std::get<int *>(prop.value);
        int oldValue = *intPtr;

        if (prop.readOnly)
        {
            ImGui::BeginDisabled();
        }

        if (prop.hasRange)
        {
            if (ImGui::SliderInt(prop.name.c_str(), intPtr, static_cast<int>(prop.minValue),
                                 static_cast<int>(prop.maxValue)))
            {
                if (*intPtr != oldValue)
                {
                    notifyPropertyChanged(prop.name);
                }
            }
        }
        else
        {
            if (ImGui::DragInt(prop.name.c_str(), intPtr, 1.0f))
            {
                if (*intPtr != oldValue)
                {
                    notifyPropertyChanged(prop.name);
                }
            }
        }

        if (prop.readOnly)
        {
            ImGui::EndDisabled();
        }
    }
}

void ObjectInspector::drawBooleanProperty(const ivf::Property &prop)
{
    bool *boolPtr = std::get<bool *>(prop.value);
    bool oldValue = *boolPtr;

    if (prop.readOnly)
    {
        ImGui::BeginDisabled();
    }

    if (ImGui::Checkbox(prop.name.c_str(), boolPtr))
    {
        if (*boolPtr != oldValue)
        {
            notifyPropertyChanged(prop.name);
        }
    }

    if (prop.readOnly)
    {
        ImGui::EndDisabled();
    }
}

void ObjectInspector::drawStringProperty(const ivf::Property &prop)
{
    std::string *stringPtr = std::get<std::string *>(prop.value);

    // Create a buffer for ImGui input
    static char buffer[256];
    strncpy(buffer, stringPtr->c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    if (prop.readOnly)
    {
        ImGui::BeginDisabled();
    }

    if (ImGui::InputText(prop.name.c_str(), buffer, sizeof(buffer)))
    {
        *stringPtr = std::string(buffer);
        notifyPropertyChanged(prop.name);
    }

    if (prop.readOnly)
    {
        ImGui::EndDisabled();
    }
}

bool ObjectInspector::isCategoryExpanded(const std::string &category) const
{
    return std::find(m_expandedCategories.begin(), m_expandedCategories.end(), category) != m_expandedCategories.end();
}

void ObjectInspector::setCategoryExpanded(const std::string &category, bool expanded)
{
    auto it = std::find(m_expandedCategories.begin(), m_expandedCategories.end(), category);

    if (expanded && it == m_expandedCategories.end())
    {
        m_expandedCategories.push_back(category);
    }
    else if (!expanded && it != m_expandedCategories.end())
    {
        m_expandedCategories.erase(it);
    }
}

void ObjectInspector::notifyPropertyChanged(const std::string &propertyName)
{
    if (m_currentObject)
    {
        m_currentObject->notifyPropertyChanged(propertyName);
    }
}

std::string ObjectInspector::getPropertyId(const ivf::Property &prop) const
{
    return prop.name + "_" + prop.category;
}
