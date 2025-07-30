#include <ivfui/effect_inspector.h>

#include <ivf/property_inspectable.h>

#include <imgui.h>
#include <algorithm>

using namespace ivfui;

EffectInspector::EffectInspector(const std::string &name) : UiWindow(name), m_effectProvider(nullptr)
{
    this->setSize(400, 600);
}

EffectInspector::EffectInspector(const std::string &name, EffectListProvider *effectProvider)
    : UiWindow(name), m_effectProvider(effectProvider)
{
    this->setSize(400, 600);
}

EffectInspector::~EffectInspector()
{}

std::shared_ptr<EffectInspector> EffectInspector::create(const std::string &name)
{
    return std::make_shared<EffectInspector>(name);
}

std::shared_ptr<EffectInspector> EffectInspector::create(const std::string &name, EffectListProvider *effectProvider)
{
    return std::make_shared<EffectInspector>(name, effectProvider);
}

void EffectInspector::setEffectProvider(EffectListProvider *effectProvider)
{
    m_effectProvider = effectProvider;
    // Clear selection when effect provider changes
    m_selectedEffect = nullptr;
}

EffectListProvider *EffectInspector::effectProvider() const
{
    return m_effectProvider;
}

void EffectInspector::setSelectedEffect(std::shared_ptr<ivf::Effect> effect)
{
    m_selectedEffect = effect;
    if (m_onEffectSelected && effect)
        m_onEffectSelected(effect);
}

std::shared_ptr<ivf::Effect> EffectInspector::selectedEffect() const
{
    return m_selectedEffect;
}

void EffectInspector::setOnEffectSelected(std::function<void(std::shared_ptr<ivf::Effect>)> callback)
{
    m_onEffectSelected = callback;
}

void EffectInspector::setShowProperties(bool show)
{
    m_showProperties = show;
}

bool EffectInspector::showProperties() const
{
    return m_showProperties;
}

void EffectInspector::setShowAdvancedProperties(bool show)
{
    m_showAdvancedProperties = show;
}

bool EffectInspector::showAdvancedProperties() const
{
    return m_showAdvancedProperties;
}

void EffectInspector::setShowOptions(bool show)
{
    m_showOptions = show;
}

bool EffectInspector::showOptions() const
{
    return m_showOptions;
}

void EffectInspector::doDraw()
{
    if (!m_effectProvider)
    {
        ImGui::TextWrapped("No effect provider set. Use setEffectProvider() to set the effect provider.");
        return;
    }

    // Draw inspector options in a collapsible header
    if (m_showOptions)
    {
        if (ImGui::CollapsingHeader("Inspector Options"))
        {
            ImGui::Indent();
            drawInspectorOptions();
            ImGui::Unindent();
            ImGui::Spacing();
        }

        ImGui::Separator();
        ImGui::Spacing();
    }

    // Main content area with splitter
    int effectCount = getEffectCount();

    if (effectCount > 0)
    {
        // Calculate available content size
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        float bottomSpacing = ImGui::GetFrameHeightWithSpacing(); // Space for selection info
        float availableHeight = contentSize.y - bottomSpacing;

        if (m_showProperties)
        {
            // Split-pane layout with manual positioning
            float listWidth = contentSize.x * m_splitterPosition;

            // Effects list on the left
            ImGui::BeginChild("EffectsList", ImVec2(listWidth - 4, availableHeight), true,
                              ImGuiWindowFlags_HorizontalScrollbar);

            drawEffectsList();

            ImGui::EndChild();

            // Splitter
            ImGui::SameLine();
            ImGui::Button("##splitter", ImVec2(8.0f, availableHeight));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                float delta = ImGui::GetIO().MouseDelta.x / contentSize.x;
                m_splitterPosition += delta;
                m_splitterPosition = std::max(0.2f, std::min(0.8f, m_splitterPosition));
            }
            if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            // Properties panel on the right - let it fill remaining space
            ImGui::SameLine();
            ImGui::BeginChild("PropertiesPanel", ImVec2(-1, availableHeight), true); // -1 means fill remaining width
            drawPropertiesPanel();
            ImGui::EndChild();
        }
        else
        {
            // Single-pane layout: just the effects list
            ImGui::BeginChild("EffectsList", ImVec2(0, availableHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
            drawEffectsList();
            ImGui::EndChild();
        }

        // Show selection info at the bottom
        ImGui::Spacing();
        if (m_selectedEffect)
        {
            ImGui::Text("Selected: %s", getEffectDisplayName(m_selectedEffect, -1).c_str());
        }
        else
        {
            ImGui::TextDisabled("No effect selected");
        }
    }
    else
    {
        ImGui::TextWrapped("No post-processing effects available. Add effects to the effect provider to inspect them.");
    }
}

ImGuiWindowFlags EffectInspector::doWindowFlags() const
{
    return ImGuiWindowFlags_None;
}

void EffectInspector::drawEffectsList()
{
    ImGui::Text("Post-Processing Effects:");
    ImGui::Spacing();

    // Control buttons
    if (ImGui::Button("Enable All"))
    {
        for (int i = 0; i < getEffectCount(); ++i)
        {
            m_effectProvider->enableEffect(i);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Disable All"))
    {
        m_effectProvider->disableAllEffects();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All"))
    {
        m_effectProvider->clearEffects();
        m_selectedEffect = nullptr;
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Effects list
    for (int i = 0; i < getEffectCount(); ++i)
    {
        auto effect = getEffect(i);
        if (effect)
        {
            drawEffectItem(effect, i);
        }
    }
}

bool EffectInspector::drawEffectItem(std::shared_ptr<ivf::Effect> effect, int index)
{
    if (!effect)
        return false;

    std::string displayName = getEffectDisplayName(effect, index);
    bool isEnabled = m_effectProvider->isEffectEnabled(index);
    bool isSelected = (effect == m_selectedEffect);

    // Create a selectable item with checkbox for enabled state
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (isSelected)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Effect item with enable/disable checkbox
    ImGui::PushID(index);

    // Checkbox for enabling/disabling the effect
    if (ImGui::Checkbox("##enabled", &isEnabled))
    {
        if (isEnabled)
            m_effectProvider->enableEffect(index);
        else
            m_effectProvider->disableEffect(index);
    }

    ImGui::SameLine();

    // Selectable item for the effect name
    bool itemClicked = ImGui::TreeNodeEx(effect.get(), flags, "%s", effect->name().c_str());

    // Handle selection
    if (ImGui::IsItemClicked())
    {
        setSelectedEffect(effect);
    }

    // Context menu for right-click
    if (ImGui::BeginPopupContextItem())
    {
        ImGui::Text("Effect: %s", effect->name().c_str());
        ImGui::Separator();

        if (ImGui::MenuItem("Enable", nullptr, isEnabled))
        {
            if (!isEnabled)
                m_effectProvider->enableEffect(index);
        }

        if (ImGui::MenuItem("Disable", nullptr, !isEnabled))
        {
            if (isEnabled)
                m_effectProvider->disableEffect(index);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Select"))
        {
            setSelectedEffect(effect);
        }

        // Reordering options (if supported by the provider)
        if (index > 0 && ImGui::MenuItem("Move Up"))
        {
            m_effectProvider->reorderEffect(index, index - 1);
        }

        if (index < getEffectCount() - 1 && ImGui::MenuItem("Move Down"))
        {
            m_effectProvider->reorderEffect(index, index + 1);
        }

        ImGui::EndPopup();
    }

    ImGui::PopID();

    return itemClicked;
}

std::string EffectInspector::getEffectDisplayName(std::shared_ptr<ivf::Effect> effect, int index) const
{
    if (!effect)
        return "null";

    std::string displayName;

    displayName = effect->name();

    return displayName;
}

void EffectInspector::drawInspectorOptions()
{
    // Organize checkboxes in a more spacious layout
    ImGui::Checkbox("Show Properties Panel", &m_showProperties);

    if (m_showProperties)
    {
        ImGui::Checkbox("Show Advanced Properties", &m_showAdvancedProperties);
        ImGui::SliderFloat("Drag Speed", &m_dragSpeed, 0.01f, 1.0f, "%.2f");
    }
}

void EffectInspector::drawPropertiesPanel()
{
    if (!m_selectedEffect)
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No effect selected");
        ImGui::Separator();
        ImGui::Text("Select an effect to inspect its properties");
        return;
    }

    // Effect header
    ImGui::Text("Effect: %s", getEffectDisplayName(m_selectedEffect, -1).c_str());
    ImGui::Separator();

    // Check if the effect implements PropertyInspectable
    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
    if (!inspectable)
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Effect does not support property inspection");
        return;
    }

    // Initialize properties if needed
    inspectable->initializeProperties();

    // Get all properties
    const auto &properties = inspectable->getProperties();

    if (properties.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "No properties to display");
        return;
    }

    // Draw all properties directly without categories
    for (const auto &prop : properties)
    {
        ImGui::PushID(getPropertyId(prop).c_str());
        drawProperty(prop);
        ImGui::PopID();
    }
}

void EffectInspector::drawProperty(const ivf::Property &prop)
{
    std::string type = ivf::PropertyEditor::getPropertyType(prop);

    // Skip read-only properties if not showing advanced
    if (prop.readOnly && !m_showAdvancedProperties)
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
    if (type == "vec3" || type == "vec4" || type == "uvec3" || type == "uvec4")
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

void EffectInspector::drawVectorProperty(const ivf::Property &prop)
{
    std::string type = ivf::PropertyEditor::getPropertyType(prop);
    int componentCount = (type == "vec3" || type == "uvec3") ? 3 : 4;
    bool isUnsigned = (type == "uvec3" || type == "uvec4");

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
            float minVal = static_cast<float>(prop.minValue);
            float maxVal = static_cast<float>(prop.maxValue);

            // For unsigned types, ensure minimum is 0 or greater
            if (isUnsigned && minVal < 0.0f)
                minVal = 0.0f;

            if (ImGui::SliderFloat(componentId.c_str(), &value, minVal, maxVal, "%.0f"))
            {
                if (value != oldValue)
                {
                    // For unsigned types, clamp to non-negative values
                    if (isUnsigned && value < 0.0f)
                        value = 0.0f;

                    ivf::PropertyEditor::setComponentValue(prop, i, value);
                    changed = true;
                }
            }
        }
        else
        {
            // Use appropriate format string for unsigned vs signed
            const char *format = isUnsigned ? "%.0f" : "%.3f";

            if (ImGui::DragFloat(componentId.c_str(), &value, m_dragSpeed, isUnsigned ? 0.0f : -FLT_MAX, FLT_MAX,
                                 format))
            {
                if (value != oldValue)
                {
                    // For unsigned types, clamp to non-negative values
                    if (isUnsigned && value < 0.0f)
                        value = 0.0f;

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
        // Notify the selected effect that a property has changed
        auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
        if (inspectable)
        {
            inspectable->notifyPropertyChanged(prop.name);
        }
    }
}

void EffectInspector::drawScalarProperty(const ivf::Property &prop)
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

                // Notify change
                auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
                if (inspectable)
                {
                    inspectable->notifyPropertyChanged(prop.name);
                }
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

                // Notify change
                auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
                if (inspectable)
                {
                    inspectable->notifyPropertyChanged(prop.name);
                }
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
                    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
                    if (inspectable)
                    {
                        inspectable->notifyPropertyChanged(prop.name);
                    }
                }
            }
        }
        else
        {
            if (ImGui::DragInt(prop.name.c_str(), intPtr, 1.0f))
            {
                if (*intPtr != oldValue)
                {
                    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
                    if (inspectable)
                    {
                        inspectable->notifyPropertyChanged(prop.name);
                    }
                }
            }
        }

        if (prop.readOnly)
        {
            ImGui::EndDisabled();
        }
    }
    else if (type == "uint")
    {
        glm::uint *uintPtr = std::get<glm::uint *>(prop.value);
        glm::uint oldValue = *uintPtr;

        // Convert to int for ImGui (which doesn't have unsigned int controls)
        int intValue = static_cast<int>(*uintPtr);

        if (prop.readOnly)
        {
            ImGui::BeginDisabled();
        }

        if (prop.hasRange)
        {
            int minVal = static_cast<int>(std::max(0.0, prop.minValue));
            int maxVal = static_cast<int>(prop.maxValue);

            if (ImGui::SliderInt(prop.name.c_str(), &intValue, minVal, maxVal))
            {
                // Ensure non-negative
                intValue = std::max(0, intValue);
                glm::uint newValue = static_cast<glm::uint>(intValue);

                if (newValue != oldValue)
                {
                    *uintPtr = newValue;
                    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
                    if (inspectable)
                    {
                        inspectable->notifyPropertyChanged(prop.name);
                    }
                }
            }
        }
        else
        {
            if (ImGui::DragInt(prop.name.c_str(), &intValue, 1.0f, 0, INT_MAX))
            {
                // Ensure non-negative
                intValue = std::max(0, intValue);
                glm::uint newValue = static_cast<glm::uint>(intValue);

                if (newValue != oldValue)
                {
                    *uintPtr = newValue;
                    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
                    if (inspectable)
                    {
                        inspectable->notifyPropertyChanged(prop.name);
                    }
                }
            }
        }

        if (prop.readOnly)
        {
            ImGui::EndDisabled();
        }
    }
}

void EffectInspector::drawBooleanProperty(const ivf::Property &prop)
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
            auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
            if (inspectable)
            {
                inspectable->notifyPropertyChanged(prop.name);
            }
        }
    }

    if (prop.readOnly)
    {
        ImGui::EndDisabled();
    }
}

void EffectInspector::drawStringProperty(const ivf::Property &prop)
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
        auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedEffect);
        if (inspectable)
        {
            inspectable->notifyPropertyChanged(prop.name);
        }
    }

    if (prop.readOnly)
    {
        ImGui::EndDisabled();
    }
}

bool EffectInspector::isCategoryExpanded(const std::string &category) const
{
    // This method is no longer used since we removed category-based display
    return true;
}

void EffectInspector::setCategoryExpanded(const std::string &category, bool expanded)
{
    // This method is no longer used since we removed category-based display
    // No-op to maintain compatibility
}

std::string EffectInspector::getPropertyId(const ivf::Property &prop) const
{
    // Simplified since we no longer need category in the ID
    return prop.name;
}

int EffectInspector::getEffectCount() const
{
    if (!m_effectProvider)
        return 0;

    return m_effectProvider->getEffectCount();
}

std::shared_ptr<ivf::Effect> EffectInspector::getEffect(int index) const
{
    if (!m_effectProvider || index < 0)
        return nullptr;

    return m_effectProvider->getEffect(index);
}
