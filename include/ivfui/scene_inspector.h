#pragma once

/**
 * @file scene_inspector.h
 * @brief Declares the SceneInspector class for hierarchical scene graph visualization.
 */

#include <ivfui/ui_window.h>
#include <ivf/node.h>
#include <ivf/composite_node.h>
#include <ivf/property_inspectable.h>

#include <memory>
#include <functional>
#include <algorithm>

namespace ivfui {

/**
 * @class SceneInspector
 * @brief ImGui-based hierarchical scene graph inspector window.
 *
 * The SceneInspector class provides a tree view of the scene graph hierarchy,
 * allowing users to visualize, select, and interact with nodes in the scene.
 * It displays node names, types, visibility status, and provides selection
 * callbacks for integration with other editor components.
 */
class SceneInspector : public UiWindow {
private:
    std::shared_ptr<ivf::Node> m_rootNode{nullptr};                   ///< Root node of the scene to inspect.
    std::shared_ptr<ivf::Node> m_selectedNode{nullptr};               ///< Currently selected node.
    std::function<void(std::shared_ptr<ivf::Node>)> m_onNodeSelected; ///< Callback for node selection.
    bool m_showInvisibleNodes{true};                                  ///< Whether to show invisible nodes in the tree.
    bool m_showNodeTypes{false};                                      ///< Whether to show node type information.
    bool m_showObjectIds{false};                                      ///< Whether to show object IDs.
    int m_treeDepth{0};                            ///< Current tree traversal depth (for indentation).
    bool m_showProperties{true};                   ///< Whether to show the properties panel.
    bool m_showAdvancedProperties{false};          ///< Whether to show advanced properties in the properties panel.
    float m_dragSpeed{0.1f};                       ///< Speed for dragging numeric controls.
    float m_splitterPosition{0.4f};                ///< Position of the splitter (0.0 = all tree, 1.0 = all properties).
    std::vector<std::string> m_expandedCategories; ///< Expanded property categories in the properties panel.

public:
    /**
     * @brief Construct a SceneInspector with the given name.
     * @param name Window name.
     */
    SceneInspector(const std::string &name = "Scene Inspector");

    /**
     * @brief Virtual destructor.
     */
    virtual ~SceneInspector();

    /**
     * @brief Factory method to create a shared pointer to a SceneInspector instance.
     * @param name Window name.
     * @return std::shared_ptr<SceneInspector> New SceneInspector instance.
     */
    static std::shared_ptr<SceneInspector> create(const std::string &name = "Scene Inspector");

    /**
     * @brief Set the root node to inspect.
     * @param rootNode Root node of the scene graph.
     */
    void setRootNode(std::shared_ptr<ivf::Node> rootNode);

    /**
     * @brief Get the current root node.
     * @return std::shared_ptr<ivf::Node> Current root node.
     */
    std::shared_ptr<ivf::Node> rootNode() const;

    /**
     * @brief Set the currently selected node.
     * @param node Node to select.
     */
    void setSelectedNode(std::shared_ptr<ivf::Node> node);

    /**
     * @brief Get the currently selected node.
     * @return std::shared_ptr<ivf::Node> Currently selected node.
     */
    std::shared_ptr<ivf::Node> selectedNode() const;

    /**
     * @brief Set callback function for when a node is selected.
     * @param callback Function to call when node selection changes.
     */
    void setOnNodeSelected(std::function<void(std::shared_ptr<ivf::Node>)> callback);

    /**
     * @brief Set whether to show invisible nodes in the tree.
     * @param show True to show invisible nodes, false to hide them.
     */
    void setShowInvisibleNodes(bool show);

    /**
     * @brief Check if invisible nodes are shown.
     * @return bool True if invisible nodes are shown.
     */
    bool showInvisibleNodes() const;

    /**
     * @brief Set whether to show node type information.
     * @param show True to show node types, false to hide them.
     */
    void setShowNodeTypes(bool show);

    /**
     * @brief Check if node types are shown.
     * @return bool True if node types are shown.
     */
    bool showNodeTypes() const;

    /**
     * @brief Set whether to show object IDs.
     * @param show True to show object IDs, false to hide them.
     */
    void setShowObjectIds(bool show);

    /**
     * @brief Check if object IDs are shown.
     * @return bool True if object IDs are shown.
     */
    bool showObjectIds() const;

    /**
     * @brief Set whether to show the properties panel.
     * @param show True to show properties panel, false to hide it.
     */
    void setShowProperties(bool show);

    /**
     * @brief Check if the properties panel is shown.
     * @return bool True if properties panel is shown.
     */
    bool showProperties() const;

protected:
    /**
     * @brief Draw the scene inspector window contents.
     */
    virtual void doDraw() override;
    virtual ImGuiWindowFlags doWindowFlags() const override;

private:
    /**
     * @brief Recursively draw a node and its children in the tree view.
     * @param node Node to draw.
     * @param isRoot Whether this is the root node.
     */
    void drawNodeTree(std::shared_ptr<ivf::Node> node, bool isRoot = false);

    /**
     * @brief Draw a single node item in the tree.
     * @param node Node to draw.
     * @param hasChildren Whether the node has children.
     * @param isRoot Whether this is the root node.
     * @return bool True if the node was selected.
     */
    bool drawNodeItem(std::shared_ptr<ivf::Node> node, bool hasChildren, bool isRoot = false);

    /**
     * @brief Get a display name for a node.
     * @param node Node to get name for.
     * @return std::string Display name for the node.
     */
    std::string getNodeDisplayName(std::shared_ptr<ivf::Node> node) const;

    /**
     * @brief Get the type name of a node.
     * @param node Node to get type for.
     * @return std::string Type name of the node.
     */
    std::string getNodeTypeName(std::shared_ptr<ivf::Node> node) const;

    /**
     * @brief Check if a node has children.
     * @param node Node to check.
     * @return bool True if the node has children.
     */
    bool nodeHasChildren(std::shared_ptr<ivf::Node> node) const;

    /**
     * @brief Get the children of a node.
     * @param node Node to get children for.
     * @return std::vector<std::shared_ptr<ivf::Node>> Vector of child nodes.
     */
    std::vector<std::shared_ptr<ivf::Node>> getNodeChildren(std::shared_ptr<ivf::Node> node) const;

    /**
     * @brief Draw the inspector options/settings.
     */
    void drawInspectorOptions();

    /**
     * @brief Draw the properties panel for the selected node.
     */
    void drawPropertiesPanel();

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
     * @brief Generate a unique ImGui ID for a property.
     * @param prop Property to generate ID for.
     * @return std::string Unique ImGui ID.
     */
    std::string getPropertyId(const ivf::Property &prop) const;
};

/**
 * @typedef SceneInspectorPtr
 * @brief Shared pointer type for SceneInspector.
 */
typedef std::shared_ptr<SceneInspector> SceneInspectorPtr;

} // namespace ivfui
