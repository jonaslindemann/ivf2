#include <ivfui/scene_inspector.h>

#include <ivf/mesh_node.h>
#include <ivf/transform_node.h>

#include <imgui.h>
#include <typeinfo>

using namespace ivfui;

SceneInspector::SceneInspector(const std::string &name) : UiWindow(name)
{}

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

void SceneInspector::doDraw()
{
    // Draw inspector options in a collapsible header
    if (ImGui::CollapsingHeader("Inspector Options", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();
        drawInspectorOptions();
        ImGui::Unindent();
        ImGui::Spacing();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Draw the scene tree
    if (m_rootNode)
    {
        ImGui::Text("Scene Graph:");
        ImGui::Spacing();

        // Give the tree view more space and a border
        ImGui::BeginChild("SceneTree", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true,
                          ImGuiWindowFlags_HorizontalScrollbar);

        m_treeDepth = 0;
        drawNodeTree(m_rootNode, true);

        ImGui::EndChild();

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
        return "Mesh";
    else if (std::dynamic_pointer_cast<ivf::CompositeNode>(node))
        return "Composite";
    else if (std::dynamic_pointer_cast<ivf::TransformNode>(node))
        return "Transform";
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
}
