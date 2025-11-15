#pragma once
/**
 * @file node.h
 * @brief Node class for the ivf library.
 */

#include <ivf/glbase.h>
#include <ivf/material.h>
#include <ivf/texture.h>
#include <ivf/node_visitor.h>
#include <ivf/property_inspectable.h>

#include <memory>
#include <vector>

namespace ivf {

/**
 * @class Node
 * @brief Base class for all drawable scene nodes in the ivf library.
 *
 * The Node class defines a common interface for all drawable objects in the ivf library.
 * A node can have a material and a texture, be visible or invisible, and be drawn in
 * normal or selected state. Nodes support hierarchical relationships (parent/child),
 * object IDs for selection, and property inspection for editor integration.
 * 
 * Supports both single texture (backward compatible) and multitexturing (up to 8 textures).
 */
class Node : public GLBase, public std::enable_shared_from_this<Node>, public PropertyInspectable {
private:
    std::shared_ptr<Material> m_material{nullptr}; ///< Material associated with the node.
    std::shared_ptr<Texture> m_texture{nullptr};   ///< Primary texture (backward compatibility).
    std::vector<std::shared_ptr<Texture>> m_textures; ///< Multiple textures for multitexturing.
    bool m_useMaterial{true};                      ///< Whether to use the material for rendering.
    bool m_useTexture{false};                      ///< Whether to use the texture for rendering.
    bool m_useMultiTexturing{false};               ///< Whether to use multitexturing.
    bool m_visible{true};                          ///< Whether the node is visible.
    uint32_t m_objectId{0};                        ///< Object ID for selection.
    std::weak_ptr<Node> m_parent{};                ///< Parent node (for hierarchy).
    std::string m_name;                            ///< Name of the node (for identification).

public:
    /**
     * @brief Get the parent node.
     * @return std::shared_ptr<Node> Parent node pointer.
     */
    std::shared_ptr<Node> parent() const;

    /**
     * @brief Set the parent node.
     * @param parent Parent node pointer.
     */
    void setParent(std::shared_ptr<Node> parent);

    /**
     * @brief Draw the node to the screen.
     */
    void draw();

    /**
     * @brief Draw the node in a selected state.
     */
    void drawSelection();

    /**
     * @brief Set the material for the node.
     * @param material Shared pointer to the material.
     */
    void setMaterial(std::shared_ptr<Material> material);

    /**
     * @brief Get the current material for the node.
     * @return std::shared_ptr<Material> Material pointer.
     */
    std::shared_ptr<Material> material();

    /**
     * @brief Set the texture for the node (single texture, backward compatible).
     * @param texture Shared pointer to the texture.
     */
    void setTexture(std::shared_ptr<Texture> texture);

    /**
     * @brief Get the current texture for the node.
     * @return std::shared_ptr<Texture> Texture pointer.
     */
    std::shared_ptr<Texture> texture();
    
    // Multitexturing methods
    
    /**
     * @brief Add a texture to the multitexture stack.
     * @param texture Shared pointer to the texture to add.
     */
    void addTexture(std::shared_ptr<Texture> texture);
    
    /**
     * @brief Set texture at a specific index.
     * @param index Texture index (0-7).
     * @param texture Shared pointer to the texture.
     */
    void setTexture(size_t index, std::shared_ptr<Texture> texture);
    
    /**
     * @brief Remove texture at a specific index.
     * @param index Texture index to remove.
     */
    void removeTexture(size_t index);
    
    /**
     * @brief Clear all textures.
     */
    void clearTextures();
    
    /**
     * @brief Get texture at a specific index.
     * @param index Texture index.
     * @return std::shared_ptr<Texture> Texture pointer or nullptr.
     */
    std::shared_ptr<Texture> getTexture(size_t index);
    
    /**
     * @brief Get the number of active textures.
     * @return size_t Number of textures.
     */
    size_t textureCount() const;
    
    /**
     * @brief Get all textures.
     * @return const std::vector<std::shared_ptr<Texture>>& Reference to texture vector.
     */
    const std::vector<std::shared_ptr<Texture>>& textures() const;
    
    /**
     * @brief Enable or disable multitexturing.
     * @param flag True to enable multitexturing, false for single texture mode.
     */
    void setUseMultiTexturing(bool flag);
    
    /**
     * @brief Check if multitexturing is enabled.
     * @return bool True if multitexturing is enabled.
     */
    bool useMultiTexturing() const;

    /**
     * @brief Enable or disable the use of the material for rendering.
     * @param flag True to use the material, false otherwise.
     */
    void setUseMaterial(bool flag);

    /**
     * @brief Check if the material is used for rendering.
     * @return bool True if the material is used.
     */
    bool useMaterial();

    /**
     * @brief Enable or disable the use of the texture for rendering.
     * @param flag True to use the texture, false otherwise.
     */
    void setUseTexture(bool flag);

    /**
     * @brief Check if the texture is used for rendering.
     * @return bool True if the texture is used.
     */
    bool useTexture();

    /**
     * @brief Set the visibility of the node.
     * @param flag True to make visible, false to hide.
     */
    void setVisible(bool flag);

    /**
     * @brief Check if the node is visible.
     * @return bool True if visible.
     */
    bool visible() const;

    /**
     * @brief Set the object ID for the node (used for selection).
     * @param objectId Object ID value.
     */
    void setObjectId(uint32_t objectId);

    /**
     * @brief Get the current object ID for the node.
     * @return uint32_t Object ID value.
     */
    uint32_t objectId() const;

    /**
     * @brief Set the name of the node.
     * @param name Name string.
     */
    void setName(const std::string &name);

    /**
     * @brief Get the name of the node.
     * @return std::string Name of the node.
     */
    std::string name() const;

    /**
     * @brief Enumerate and assign the next object ID for the node (used for selection).
     * @param startId Starting object ID.
     * @return uint32_t Next available object ID.
     */
    uint32_t enumerateIds(uint32_t startId);

    /**
     * @brief Accept a visitor for traversal or processing (visitor pattern).
     * @param visitor Pointer to the NodeVisitor.
     */
    virtual void accept(NodeVisitor *visitor);

protected:
    /**
     * @brief Bind textures to OpenGL (single or multi).
     */
    void bindTextures();

    /**
     * @brief Called before drawing the node. Override to perform actions before drawing.
     */
    virtual void doPreDraw();

    /**
     * @brief Called to perform the actual drawing of the node. Override to implement custom drawing.
     */
    virtual void doDraw();

    /**
     * @brief Called after drawing the node. Override to perform actions after drawing.
     */
    virtual void doPostDraw();

    /**
     * @brief Called to perform the actual drawing of the node in a selected state.
     */
    virtual void doDrawSelection();

    /**
     * @brief Called to set up the node's properties. Override to implement custom setup.
     */
    virtual void doSetup();

    /**
     * @brief Called to enumerate object IDs. Override to implement custom ID enumeration.
     * The default implementation assigns the startId to the object ID and returns the next ID.
     * @param startId Starting object ID.
     * @return uint32_t Next available object ID.
     */
    virtual uint32_t doEnumerateIds(uint32_t startId);

    /**
     * @brief Register properties for inspection (editor integration).
     */
    virtual void setupProperties() override;
};

/**
 * @typedef NodePtr
 * @brief Shared pointer type for Node.
 */
typedef std::shared_ptr<Node> NodePtr;

} // namespace ivf
