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
#include <ivf/behavior.h>
#include <ivf/render_queue.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <type_traits>

namespace ivf {

class RenderContext;
struct DrawContext;
struct Ray;
struct RayHit;

template <typename T>
concept ValidNodeType = std::is_base_of_v<Node, T>;

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
class Node : public GLBase, public std::enable_shared_from_this<Node>, public PropertyInspectable, public BehaviorHost {
private:
    std::shared_ptr<Material> m_material{nullptr}; ///< Material associated with the node.
    std::shared_ptr<Texture> m_texture{nullptr};   ///< Primary texture (backward compatibility).
    std::vector<std::shared_ptr<Texture>> m_textures; ///< Multiple textures for multitexturing.
    bool m_useMaterial{true};                      ///< Whether to use the material for rendering.
    bool m_useTexture{false};                      ///< Whether to use the texture for rendering.
    bool m_useMultiTexturing{false};               ///< Whether to use multitexturing.
    bool m_visible{true};                          ///< Whether the node is visible.
    bool m_castShadows{true};                      ///< Whether the node is drawn into shadow maps.
uint32_t m_objectId{0};                        ///< Object ID for selection.
    std::weak_ptr<Node> m_parent{};                ///< Parent node (for hierarchy).
    std::string m_name;                            ///< Name of the node (for identification).
    std::optional<RenderLayer> m_renderLayer;      ///< Explicit draw order group (see setRenderLayer()).

    friend class Renderer; // calls the draw hooks of collected items

public:
    /**
     * @brief Get the parent node.
     * @return std::shared_ptr<Node> Parent node pointer.
     */
    [[nodiscard]] std::shared_ptr<Node> parent() const;

    /**
     * @brief Set the parent node.
     * @param parent Parent node pointer.
     */
    void setParent(std::shared_ptr<Node> parent);

    /**
     * @brief Draw the node (and, for containers, its children) to the screen.
     *
     * Collects the node into a RenderQueue (see collect()) and draws it with a Renderer,
     * starting from the current model matrix. Uses the current RenderContext; see
     * draw(RenderContext &).
     */
    void draw();

    /**
     * @brief Draw the node (and, for containers, its children) into @p context.
     *
     * Like draw(), with an explicit render context (RenderPass::Main, no culling). The
     * context's OpenGL context must be current.
     * @param context Render context to draw into.
     */
    void draw(RenderContext &context);

    /**
     * @brief Add this node's draw items to @p queue.
     *
     * The default adds one DrawItem for a visible node; the Renderer then calls its
     * doPreDraw()/doDraw()/doPostDraw(). Containers (CompositeNode, InstanceNode) override
     * this to collect their children instead. Override it to add several items or to
     * decide per frame what to draw.
     * @param queue Queue receiving the items.
     * @param state Transform, material and textures inherited from the ancestors.
     */
    virtual void collect(RenderQueue &queue, const CollectState &state);

    /**
     * @brief Put the node in a draw order group regardless of its material.
     *
     * Without it the layer follows the node's (or inherited) material: Transparent for
     * alpha < 1 and blended materials, Background for background blending, else Opaque.
     * @param layer Group the node's draw items belong to.
     */
    void setRenderLayer(RenderLayer layer);

    /** @brief Let the material decide the layer again (undoes setRenderLayer()). */
    void clearRenderLayer();

    /** @brief The explicitly set layer, if any. */
    [[nodiscard]] std::optional<RenderLayer> renderLayer() const;

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
    [[nodiscard]] std::shared_ptr<Material> material();

    /**
     * @brief Set the texture for the node (single texture, backward compatible).
     * @param texture Shared pointer to the texture.
     */
    void setTexture(std::shared_ptr<Texture> texture);

    /**
     * @brief Get the current texture for the node.
     * @return std::shared_ptr<Texture> Texture pointer.
     */
    [[nodiscard]] std::shared_ptr<Texture> texture();
    
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
    [[nodiscard]] std::shared_ptr<Texture> texture(size_t index);
    [[deprecated("Use texture() instead")]] std::shared_ptr<Texture> getTexture(size_t index) { return texture(index); }
    
    /**
     * @brief Get the number of active textures.
     * @return size_t Number of textures.
     */
    [[nodiscard]] inline size_t textureCount() const noexcept { return m_textures.size(); }
    
    /**
     * @brief Get all textures as a span (modern C++20 interface).
     * @return std::span<const std::shared_ptr<Texture>> Read-only span of textures.
     * 
     * Example usage:
     * @code
     * for (const auto& tex : node->textures()) {
     *     if (tex) tex->bind();
     * }
     * @endcode
     */
    [[nodiscard]] inline std::span<const std::shared_ptr<Texture>> textures() const noexcept { 
        return std::span<const std::shared_ptr<Texture>>(m_textures); 
    }
    
    /**
     * @brief Get all textures as a mutable span (modern C++20 interface).
     * @return std::span<std::shared_ptr<Texture>> Mutable span of textures.
     * 
     * @note Use with caution. Modifying the underlying vector structure 
     * (e.g., adding/removing elements) will invalidate the span.
     */
    [[nodiscard]] inline std::span<std::shared_ptr<Texture>> texturesMutable() noexcept { 
        return std::span<std::shared_ptr<Texture>>(m_textures); 
    }
    
    /**
     * @brief Enable or disable multitexturing.
     * @param flag True to enable multitexturing, false for single texture mode.
     */
    inline void setUseMultiTexturing(bool flag) noexcept { m_useMultiTexturing = flag; }
    
    /**
     * @brief Check if multitexturing is enabled.
     * @return bool True if multitexturing is enabled.
     */
    [[nodiscard]] inline bool useMultiTexturing() const noexcept { return m_useMultiTexturing; }

    /**
     * @brief Enable or disable the use of the material for rendering.
     * @param flag True to use the material, false otherwise.
     */
    inline void setUseMaterial(bool flag) noexcept { m_useMaterial = flag; }

    /**
     * @brief Check if the material is used for rendering.
     * @return bool True if the material is used.
     */
    [[nodiscard]] inline bool useMaterial() const noexcept { return m_useMaterial; }

    /**
     * @brief Enable or disable the use of the texture for rendering.
     * @param flag True to use the texture, false otherwise.
     */
    inline void setUseTexture(bool flag) noexcept { m_useTexture = flag; }

    /**
     * @brief Check if the texture is used for rendering.
     * @return bool True if the texture is used.
     */
    [[nodiscard]] inline bool useTexture() const noexcept { return m_useTexture; }

    /**
     * @brief Set the visibility of the node.
     * @param flag True to make visible, false to hide.
     */
    inline void setVisible(bool flag) noexcept { m_visible = flag; }

    /**
     * @brief Check if the node is visible.
     * @return bool True if visible.
     */
    [[nodiscard]] inline bool visible() const noexcept { return m_visible; }

    /**
     * @brief Draw the node (and its children) into shadow maps (default true).
     *
     * Only opaque nodes cast shadows; transparent, background and overlay layers never do.
     */
    inline void setCastShadows(bool flag) noexcept { m_castShadows = flag; }

    /** @brief Whether the node is drawn into shadow maps. */
    [[nodiscard]] inline bool castsShadows() const noexcept { return m_castShadows; }

    /**
     * @brief Ray test of nodes drawn without triangles (point clouds, ...), used by pick().
     *
     * Override it for nodes that know where they are hit better than their meshes or
     * bounding box. On a hit nearer than @p maxDistance, fill @p hit's distance, position,
     * normal (and point, for point clouds) and return true; pick() sets the node.
     * @param ray World-space ray (unit direction).
     * @param world The node's world matrix as drawn (DrawItem::world).
     * @return std::nullopt (the default) if the node has no own test: pick() then tests its
     *         meshes, or its bounding box.
     */
    [[nodiscard]] virtual std::optional<bool> intersectRay(const Ray &ray, const glm::mat4 &world, float maxDistance,
                                                           RayHit &hit) const;

    /**
     * @brief Set the object ID for the node (used for selection).
     * @param objectId Object ID value.
     */
    inline void setObjectId(uint32_t objectId) noexcept { m_objectId = objectId; }

    /**
     * @brief Get the current object ID for the node.
     * @return uint32_t Object ID value.
     */
    [[nodiscard]] inline uint32_t objectId() const noexcept { return m_objectId; }

    /**
     * @brief Set the name of the node.
     * @param name Name string.
     */
    void setName(std::string_view name);

    /**
     * @brief Get the name of the node.
     * @return std::string Name of the node.
     */
    [[nodiscard]] std::string name() const;

    /**
     * @brief Enumerate and assign the next object ID for the node (used for selection).
     * @param startId Starting object ID.
     * @return uint32_t Next available object ID.
     */
    [[nodiscard]] uint32_t enumerateIds(uint32_t startId);

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
     * @brief Unbind the textures bound by bindTextures().
     */
    void unbindTextures();

    /**
     * @brief The draw item for this node with the given inherited state (no bounds;
     * TransformNode adds them). Used by collect().
     */
    [[nodiscard]] DrawItem drawItem(const CollectState &state) const;

    /**
     * @brief State for this node's children: the node's material and textures replace the
     * inherited ones; TransformNode also applies its transform to the world matrix.
     * @param state State inherited by this node.
     */
    [[nodiscard]] virtual CollectState childState(const CollectState &state) const;

    /**
     * @brief Called before drawing the node. Override to perform actions before drawing.
     */
    virtual void doPreDraw();

    /**
     * @brief Called to perform the actual drawing of the node. Override to implement custom drawing.
     */
    virtual void doDraw();

    /**
     * @brief Draw the node for one pass of a Renderer. The default calls doDraw().
     *
     * Override it instead of doDraw() when the drawing depends on the pass (for example,
     * nothing in RenderPass::Shadow) or when the node should use the renderer's
     * RenderContext (@c context.context) instead of the manager singletons. Called between
     * doPreDraw() and doPostDraw(), only for visible nodes.
     * @param context Render context, pass, item and camera of this drawing.
     */
    virtual void doDrawPass(const DrawContext &context);

    /**
     * @brief Called after drawing the node. Override to perform actions after drawing.
     */
    virtual void doPostDraw();

    /**
     * @brief Called at the very end of doPostDraw(). Override in subclasses that need
     * to perform cleanup after textures are unbound (e.g., restore a different shader program).
     */
    virtual void onPostDraw() {}

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
using NodePtr = std::shared_ptr<Node>;

} // namespace ivf
