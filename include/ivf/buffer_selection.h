#pragma once

#include <map>

#include <ivf/glbase.h>
#include <ivf/node.h>
#include <ivf/composite_node.h>
#include <ivf/node_visitor.h>

namespace ivf {

/**
 * @class BufferSelection
 * @brief Implements object selection using an offscreen framebuffer and color picking.
 *
 * BufferSelection provides a mechanism for selecting scene objects by rendering the scene to an
 * offscreen framebuffer, where each object is drawn with a unique color ID. By reading the color
 * value at a specific pixel, the corresponding object can be identified. This is commonly used
 * for mouse picking and selection in 3D editors and viewers.
 *
 * The class manages the framebuffer, color texture, and depth buffer, and maintains a mapping
 * between object IDs and Node pointers.
 */
class BufferSelection : public GLBase {
private:
    GLuint m_fbo{0};            ///< Framebuffer object for offscreen rendering.
    GLuint m_colorTexture{0};   ///< Texture storing color IDs for selection.
    GLuint m_depthRenderBuffer; ///< Renderbuffer for depth information.

    int m_width{0};  ///< Width of the selection buffer.
    int m_height{0}; ///< Height of the selection buffer.

    CompositeNodePtr m_scene; ///< Scene graph root node for selection rendering.

    NodeMap m_nodeMap; ///< Mapping from object IDs to Node pointers.

public:
    /**
     * @brief Constructor.
     * @param scene Root composite node of the scene to be used for selection.
     */
    BufferSelection(CompositeNodePtr scene);

    /**
     * @brief Destructor.
     */
    virtual ~BufferSelection();

    /**
     * @brief Factory method to create a shared pointer to a BufferSelection instance.
     * @param scene Root composite node of the scene to be used for selection.
     * @return std::shared_ptr<BufferSelection> New BufferSelection instance.
     */
    static std::shared_ptr<BufferSelection> create(CompositeNodePtr scene);

    /**
     * @brief Initialize the selection buffer with the given dimensions.
     * @param width Buffer width in pixels.
     * @param height Buffer height in pixels.
     */
    void initialize(int width, int height);

    /**
     * @brief Resize the selection buffer.
     * @param width New buffer width in pixels.
     * @param height New buffer height in pixels.
     */
    void resize(int width, int height);

    /**
     * @brief Begin selection rendering. Binds the framebuffer and prepares for drawing.
     */
    void begin();

    /**
     * @brief Get the object ID at the specified pixel coordinates.
     * @param x X coordinate in the buffer.
     * @param y Y coordinate in the buffer.
     * @return unsigned int Object ID at the given pixel.
     */
    unsigned int idAtPixel(int x, int y);

    /**
     * @brief Get the Node pointer corresponding to a given object ID.
     * @param objectId The object ID.
     * @return Node* Pointer to the Node, or nullptr if not found.
     */
    Node *nodeFromId(unsigned int objectId);

    /**
     * @brief Get the Node pointer at the specified pixel coordinates.
     * @param x X coordinate in the buffer.
     * @param y Y coordinate in the buffer.
     * @return Node* Pointer to the Node at the pixel, or nullptr if not found.
     */
    Node *nodeAtPixel(int x, int y);

    /**
     * @brief End selection rendering. Unbinds the framebuffer.
     */
    void end();

    /**
     * @brief Clear the selection buffer and internal mappings.
     */
    void clear();
};

/**
 * @typedef BufferSelectionPtr
 * @brief Shared pointer type for BufferSelection.
 */
typedef std::shared_ptr<BufferSelection> BufferSelectionPtr;

}; // namespace ivf
