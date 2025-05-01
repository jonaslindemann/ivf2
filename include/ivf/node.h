#pragma once

/**
 * @file node.h
 * @brief Node class for the ivf library.
 */

#include <ivf/glbase.h>
#include <ivf/material.h>
#include <ivf/texture.h>
#include <ivf/node_visitor.h>

#include <memory>

namespace ivf {

/**
 * Node class for the ivf library.
 *
 * This class is used to define a common interface for all nodes in the ivf library. A node is
 * an object that can be drawn to the screen. It can have a material and a texture associated with
 * it. It can also be visible or invisible. The node can be drawn using the draw() method, and it
 * also has a special method drawSelection() for drawing it in a selected state. The node also
 * provides a special object ID which is used for selection.
 */
class Node : public GLBase, public std::enable_shared_from_this<Node> {
private:
    std::shared_ptr<Material> m_material{nullptr};
    std::shared_ptr<Texture> m_texture{nullptr};
    bool m_useMaterial{true};
    bool m_useTexture{false};
    bool m_visible{true};
    uint32_t m_objectId{0};
    std::weak_ptr<Node> m_parent{};

public:
    std::shared_ptr<Node> parent() const;
    void setParent(std::shared_ptr<Node> parent);
    /**
     * Draws the node to the screen.
     */
    void draw();

    /**
     * Draws the node in a selected state.
     */
    void drawSelection();

    /**
     * Set the material for the node.
     */
    void setMaterial(std::shared_ptr<Material> material);

    /**
     * Return the current material for the node.
     */
    std::shared_ptr<Material> material();

    /**
     * Set the texture for the node.
     */
    void setTexture(std::shared_ptr<Texture> texture);

    /**
     * Return the current texture for the node.
     */
    std::shared_ptr<Texture> texture();

    /**
     * If set to true, the node will use the material for rendering. If set to false, the node will * not use the
     * material. Default is true.
     */
    void setUseMaterial(bool flag);

    /**
     * Return the current state of the useMaterial flag.
     */
    bool useMaterial();

    /**
     * If set to true, the node will use the texture for rendering. If set to false, the node will * not use the
     * texture. Default is false.
     */
    void setUseTexture(bool flag);

    /**
     * Return the current state of the useTexture flag.
     */
    bool useTexture();

    /**
     * Set the visibility of the node. If set to true, the node will be visible. If set to false,
     * the node will be invisible. Default is true.
     */
    void setVisible(bool flag);
    bool visible() const;

    /**
     * Set the object ID for the node. The object ID is used for selection. The default value is 0.
     */
    void setObjectId(uint32_t objectId);

    /**
     * Return the current object ID for the node.
     */
    uint32_t objectId() const;

    /**
     * Return the next object ID for the node. This method is used for selection. The default value is 0.
     */
    uint32_t enumerateIds(uint32_t startId);

    /**
     * Accept a visitor. This method is used for the visitor pattern. The visitor will be able to
     * visit the node and perform some action on it.
     */
    virtual void accept(NodeVisitor *visitor);

protected:
    /**
     * Protected virtual method called before drawing the node. This method can be overridden by
     * subclasses to perform some action before drawing the node. The default implementation does
     * nothing.
     */
    virtual void doPreDraw();

    /**
     * Protected virtual method for implementing the actual drawing of the node. The default
     * implementation does nothing.
     */
    virtual void doDraw();

    /**
     * Protected virtual method called after drawing the node. This method can be overridden by
     * subclasses to perform some action after drawing the node. The default implementation does
     * nothing.
     */
    virtual void doPostDraw();

    /**
     * Protected virtual method for implementing the actual drawing of the node in a selected
     * state. The default implementation does nothing.
     */
    virtual void doDrawSelection();

    /**
     * Protected virtual method for enumerating the object IDs. This method can be overridden by
     * subclasses to implement custom ID enumeration. The default implementation assigns the startId
     * to the object ID and returns the next ID.
     */
    virtual uint32_t doEnumerateIds(uint32_t startId);
};

typedef std::shared_ptr<Node> NodePtr;

} // namespace ivf
