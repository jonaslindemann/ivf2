#include <ivfui/scene_inspector.h>

#include <ivf/mesh_node.h>
#include <ivf/transform_node.h>

#include <imgui.h>
#include <typeinfo>

using namespace ivfui;

SceneInspector::SceneInspector(const std::string &name) : UiWindow(name)
{
    this->setSize(500, 500);
}

SceneInspector::~SceneInspector()
{}

std::shared_ptr<SceneInspector> SceneInspector::create(const std::string &name)
{
    return std::make_shared<SceneInspector>(name);
}

void SceneInspector::setRootNode(std::shared_ptr<ivf::Node> rootNode)
{
    m_rootNode = rootNode;
    // Clear selection when root changes
    m_selectedNode = nullptr;
}

std::shared_ptr<ivf::Node> SceneInspector::rootNode() const
{
    return m_rootNode;
}

void SceneInspector::setSelectedNode(std::shared_ptr<ivf::Node> node)
{
    m_selectedNode = node;
    if (m_onNodeSelected && node)
        m_onNodeSelected(node);
}

std::shared_ptr<ivf::Node> SceneInspector::selectedNode() const
{
    return m_selectedNode;
}

void SceneInspector::setOnNodeSelected(std::function<void(std::shared_ptr<ivf::Node>)> callback)
{
    m_onNodeSelected = callback;
}

void SceneInspector::setShowInvisibleNodes(bool show)
{
    m_showInvisibleNodes = show;
}

bool SceneInspector::showInvisibleNodes() const
{
    return m_showInvisibleNodes;
}

void SceneInspector::setShowNodeTypes(bool show)
{
    m_showNodeTypes = show;
}

bool SceneInspector::showNodeTypes() const
{
    return m_showNodeTypes;
}

void SceneInspector::setShowObjectIds(bool show)
{
    m_showObjectIds = show;
}

bool SceneInspector::showObjectIds() const
{
    return m_showObjectIds;
}

void SceneInspector::setShowProperties(bool show)
{
    m_showProperties = show;
}

bool SceneInspector::showProperties() const
{
    return m_showProperties;
}

void SceneInspector::doDraw()
{
    // Draw inspector options in a collapsible header
    // if (ImGui::CollapsingHeader("Inspector Options", ImGuiTreeNodeFlags_DefaultOpen))
    if (ImGui::CollapsingHeader("Inspector Options"))
    {
        ImGui::Indent();
        drawInspectorOptions();
        ImGui::Unindent();
        ImGui::Spacing();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Main content area with splitter
    if (m_rootNode)
    {
        // Calculate available content size
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        float bottomSpacing = ImGui::GetFrameHeightWithSpacing(); // Space for selection info
        float availableHeight = contentSize.y - bottomSpacing;

        if (m_showProperties)
        {
            // Split-pane layout with manual positioning
            float treeWidth = contentSize.x * m_splitterPosition;

            // Scene tree on the left
            ImGui::BeginChild("SceneTree", ImVec2(treeWidth - 4, availableHeight), true,
                              ImGuiWindowFlags_HorizontalScrollbar);

            ImGui::Text("Scene Graph:");
            ImGui::Spacing();

            m_treeDepth = 0;
            drawNodeTree(m_rootNode, true);

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
            // Single-pane layout: just the tree
            ImGui::Text("Scene Graph:");
            ImGui::Spacing();

            ImGui::BeginChild("SceneTree", ImVec2(0, availableHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

            m_treeDepth = 0;
            drawNodeTree(m_rootNode, true);

            ImGui::EndChild();
        }

        // Show selection info at the bottom
        ImGui::Spacing();
        if (m_selectedNode)
        {
            ImGui::Text("Selected: %s", getNodeDisplayName(m_selectedNode).c_str());
        }
        else
        {
            ImGui::TextDisabled("No selection");
        }
    }
    else
    {
        ImGui::TextWrapped("No scene root set. Use setRootNode() to set the scene root.");
    }
}

ImGuiWindowFlags ivfui::SceneInspector::doWindowFlags() const
{
    return ImGuiWindowFlags_None;
}

void SceneInspector::drawNodeTree(std::shared_ptr<ivf::Node> node, bool isRoot)
{
    if (!node)
        return;

    // Skip invisible nodes if option is disabled
    if (!m_showInvisibleNodes && !node->visible())
        return;

    bool hasChildren = nodeHasChildren(node);
    bool nodeOpened = false;

    // Draw the node item
    if (hasChildren)
    {
        // Use TreeNode for nodes with children
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        if (node == m_selectedNode)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (isRoot)
            flags |= ImGuiTreeNodeFlags_DefaultOpen;

        std::string displayName = getNodeDisplayName(node);
        nodeOpened = ImGui::TreeNodeEx(node.get(), flags, "%s", displayName.c_str());

        // Handle selection
        if (ImGui::IsItemClicked())
        {
            setSelectedNode(node);
        }
    }
    else
    {
        // Use Selectable for leaf nodes
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        if (node == m_selectedNode)
            flags |= ImGuiTreeNodeFlags_Selected;

        std::string displayName = getNodeDisplayName(node);
        bool isSelected = ImGui::TreeNodeEx(node.get(), flags, "%s", displayName.c_str());

        // Handle selection
        if (ImGui::IsItemClicked())
        {
            setSelectedNode(node);
        }
    }

    // Context menu for right-click
    if (ImGui::BeginPopupContextItem())
    {
        ImGui::Text("Node: %s", node->name().c_str());
        ImGui::Separator();

        bool visible = node->visible();
        if (ImGui::Checkbox("Visible", &visible))
        {
            node->setVisible(visible);
        }

        if (ImGui::MenuItem("Select"))
        {
            setSelectedNode(node);
        }

        ImGui::EndPopup();
    }

    // Draw children if node is opened
    if (hasChildren && nodeOpened)
    {
        m_treeDepth++;

        auto children = getNodeChildren(node);
        for (auto child : children)
        {
            drawNodeTree(child, false);
        }

        m_treeDepth--;
        ImGui::TreePop();
    }
}

bool SceneInspector::drawNodeItem(std::shared_ptr<ivf::Node> node, bool hasChildren, bool isRoot)
{
    std::string displayName = getNodeDisplayName(node);

    ImGuiTreeNodeFlags flags = 0;

    if (!hasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (node == m_selectedNode)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (isRoot)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

    bool nodeOpen = ImGui::TreeNodeEx(node.get(), flags, "%s", displayName.c_str());

    // Handle selection
    if (ImGui::IsItemClicked())
    {
        setSelectedNode(node);
        return true;
    }

    return nodeOpen;
}

std::string SceneInspector::getNodeDisplayName(std::shared_ptr<ivf::Node> node) const
{
    if (!node)
        return "null";

    std::string displayName = node->name();

    // If no name is set, use a default
    if (displayName.empty())
        displayName = "Unnamed Node";

    // Add type information if enabled
    if (m_showNodeTypes)
    {
        std::string typeName = getNodeTypeName(node);
        displayName += " (" + typeName + ")";
    }

    // Add object ID if enabled
    if (m_showObjectIds)
    {
        displayName += " [ID: " + std::to_string(node->objectId()) + "]";
    }

    // Add visibility indicator
    if (!node->visible())
        displayName += " (Hidden)";

    return displayName;
}

std::string SceneInspector::getNodeTypeName(std::shared_ptr<ivf::Node> node) const
{
    if (!node)
        return "Unknown";

    // Try to determine the node type using dynamic_pointer_cast
    if (std::dynamic_pointer_cast<ivf::MeshNode>(node))
        return "MeshNode";
    else if (std::dynamic_pointer_cast<ivf::CompositeNode>(node))
        return "CompositeNode";
    else if (std::dynamic_pointer_cast<ivf::TransformNode>(node))
        return "TransformNode";
    else
        return "Node";
}

bool SceneInspector::nodeHasChildren(std::shared_ptr<ivf::Node> node) const
{
    if (!node)
        return false;

    // Check if it's a CompositeNode with children
    auto compositeNode = std::dynamic_pointer_cast<ivf::CompositeNode>(node);
    if (compositeNode)
    {
        return compositeNode->count() > 0;
    }

    return false;
}

std::vector<std::shared_ptr<ivf::Node>> SceneInspector::getNodeChildren(std::shared_ptr<ivf::Node> node) const
{
    std::vector<std::shared_ptr<ivf::Node>> children;

    if (!node)
        return children;

    // Get children from CompositeNode
    auto compositeNode = std::dynamic_pointer_cast<ivf::CompositeNode>(node);
    if (compositeNode)
    {
        children = compositeNode->nodes();
    }

    return children;
}

void SceneInspector::drawInspectorOptions()
{
    // Organize checkboxes in a more spacious layout
    ImGui::Checkbox("Show Invisible Nodes", &m_showInvisibleNodes);
    ImGui::Checkbox("Show Node Types", &m_showNodeTypes);
    ImGui::Checkbox("Show Object IDs", &m_showObjectIds);
    ImGui::Checkbox("Show Properties Panel", &m_showProperties);

    if (m_showProperties)
    {
        ImGui::Checkbox("Show Advanced Properties", &m_showAdvancedProperties);
        ImGui::SliderFloat("Drag Speed", &m_dragSpeed, 0.01f, 1.0f, "%.2f");
    }
}

void SceneInspector::drawPropertiesPanel()
{
    if (!m_selectedNode)
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No node selected");
        ImGui::Separator();
        ImGui::Text("Select a node to inspect its properties");
        return;
    }

    // Node header
    ImGui::Text("Node: %s", getNodeDisplayName(m_selectedNode).c_str());
    ImGui::Separator();

    // Check if the node implements PropertyInspectable
    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
    if (!inspectable)
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Node does not support property inspection");
        return;
    }

    // Initialize properties if needed
    inspectable->initializeProperties();

    // Get properties and categories
    const auto &properties = inspectable->getProperties();
    auto categories = inspectable->getCategories();

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
            auto categoryProps = inspectable->getPropertiesByCategory(category);

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

void SceneInspector::drawProperty(const ivf::Property &prop)
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

void SceneInspector::drawVectorProperty(const ivf::Property &prop)
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
        // Notify the selected node that a property has changed
        auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
        if (inspectable)
        {
            inspectable->notifyPropertyChanged(prop.name);
        }
    }
}

void SceneInspector::drawScalarProperty(const ivf::Property &prop)
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

                // Notify change
                auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
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
                auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
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
                    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
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
                    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
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
                    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
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
                    auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
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
void SceneInspector::drawBooleanProperty(const ivf::Property &prop)
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
            auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
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

void SceneInspector::drawStringProperty(const ivf::Property &prop)
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
        auto inspectable = std::dynamic_pointer_cast<ivf::PropertyInspectable>(m_selectedNode);
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

bool SceneInspector::isCategoryExpanded(const std::string &category) const
{
    return std::find(m_expandedCategories.begin(), m_expandedCategories.end(), category) != m_expandedCategories.end();
}

void SceneInspector::setCategoryExpanded(const std::string &category, bool expanded)
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

std::string SceneInspector::getPropertyId(const ivf::Property &prop) const
{
    return prop.name + "_" + prop.category;
}
