#pragma once

#include <ivf/gl.h>
#include <ivf/texture.h>

namespace ivf {

/**
 * @class FrameBuffer
 * @brief Manages an OpenGL framebuffer object for offscreen rendering and post-processing.
 *
 * The FrameBuffer class encapsulates the creation, configuration, and management of OpenGL
 * framebuffer objects (FBOs), including support for multisampling, color and depth attachments,
 * and rendering to textures. It provides methods for resizing, binding, drawing, and checking
 * framebuffer status, making it suitable for advanced rendering techniques and post-processing.
 */
class FrameBuffer : public GLBase {
private:
    GLuint m_frameBuffer;             ///< Main framebuffer object.
    GLuint m_multisampledFrameBuffer; ///< Multisampled framebuffer object.

    GLuint m_depthBuffer;             ///< Depth buffer attachment.
    GLuint m_multisampledDepthBuffer; ///< Multisampled depth buffer attachment.

    GLuint m_colorTexture;             ///< Color texture attachment.
    GLuint m_multisampledColorTexture; ///< Multisampled color texture attachment.

    GLuint m_quadVAO; ///< Vertex array object for screen quad.
    GLuint m_quadVBO; ///< Vertex buffer object for screen quad.

    bool m_multisample{false}; ///< Enable multisampling.
    int m_samples{4};          ///< Number of samples for multisampling.

    int m_width;  ///< Framebuffer width in pixels.
    int m_height; ///< Framebuffer height in pixels.

public:
    /**
     * @brief Constructor.
     * @param width Framebuffer width in pixels.
     * @param height Framebuffer height in pixels.
     */
    FrameBuffer(int width, int height);

    /**
     * @brief Destructor.
     */
    virtual ~FrameBuffer();

    /**
     * @brief Factory method to create a shared pointer to a FrameBuffer instance.
     * @param width Framebuffer width in pixels.
     * @param height Framebuffer height in pixels.
     * @return std::shared_ptr<FrameBuffer> New FrameBuffer instance.
     */
    static std::shared_ptr<FrameBuffer> create(int width, int height);

    /**
     * @brief Initialize the screen quad for post-processing.
     */
    void initQuad();

    /**
     * @brief Initialize the framebuffer and its attachments.
     */
    void initialize();

    /**
     * @brief Resize the framebuffer and its attachments.
     * @param width New width in pixels.
     * @param height New height in pixels.
     */
    void resize(int width, int height);

    /**
     * @brief Release all framebuffer resources.
     */
    void drop();

    /**
     * @brief Bind the framebuffer for rendering.
     */
    void bind();

    /**
     * @brief Unbind the framebuffer (bind default framebuffer).
     */
    void unbind();

    /**
     * @brief Attach a color texture to the framebuffer.
     * @param texture Reference to the texture ID.
     */
    void attachColorTexture(GLuint &texture);

    /**
     * @brief Attach a depth buffer to the framebuffer.
     * @param buffer Reference to the buffer ID.
     */
    void attachDepthBuffer(GLuint &buffer);

    /**
     * @brief Attach a multisampled color texture to the framebuffer.
     */
    void attachMultisampledColorTexture();

    /**
     * @brief Attach a multisampled depth buffer to the framebuffer.
     */
    void attachMultisampledDepthBuffer();

    /**
     * @brief Attach a multisampled stencil buffer to the framebuffer.
     */
    void attachMultisampledStencilBuffer();

    /**
     * @brief Enable or disable multisampling.
     * @param multisample True to enable, false to disable.
     */
    void setMultisample(bool multisample);

    /**
     * @brief Set the number of samples for multisampling.
     * @param samples Number of samples.
     */
    void setSamples(int samples);

    /**
     * @brief Check if multisampling is enabled.
     * @return bool True if multisampling is enabled.
     */
    bool multisample();

    /**
     * @brief Get the number of samples for multisampling.
     * @return int Number of samples.
     */
    int samples();

    /**
     * @brief Check the status of the framebuffer (completeness, etc.).
     */
    void checkStatus();

    /**
     * @brief Begin rendering to the framebuffer.
     */
    void begin();

    /**
     * @brief End rendering to the framebuffer.
     */
    void end();

    /**
     * @brief Draw the contents of the framebuffer (e.g., render the screen quad).
     */
    void draw();

    /**
     * @brief Get the color texture attached to the framebuffer.
     * @return GLuint Texture ID.
     */
    GLuint colorTexture();

    /**
     * @brief Get the OpenGL framebuffer object ID.
     * @return GLuint Framebuffer object ID.
     */
    GLuint id();
};

/**
 * @typedef FrameBufferPtr
 * @brief Shared pointer type for FrameBuffer.
 */
typedef std::shared_ptr<FrameBuffer> FrameBufferPtr;

}; // namespace ivf
