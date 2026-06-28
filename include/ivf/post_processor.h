#pragma once

#include <vector>

#include <ivf/gl.h>
#include <ivf/texture.h>

namespace ivf {

/**
 * @class PostProcessor
 * @brief Manages post-processing effects using framebuffer objects and shader programs.
 *
 * The PostProcessor class handles the application of post-processing effects to rendered images.
 * It manages framebuffer objects, textures, and a sequence of shader programs (effects) that can
 * be applied in order. The class supports initialization, resizing, effect management, and
 * rendering the final output.
 *
 * Texture unit conventions exposed to effect shaders:
 *   - unit 0 : the current source image, sampled via `uniform sampler2D screenTexture`.
 *   - unit 1 : the previous frame's final composite, sampled via `uniform sampler2D previousFrame`.
 *              This is what enables temporal/feedback effects (feedback, trails, motion blur).
 *              Effects that do not declare `previousFrame` simply ignore it.
 */
class PostProcessor : public GLBase {
private:
    int m_width;  ///< Width of the render target.
    int m_height; ///< Height of the render target.
    float m_time; ///< Current time (for time-based effects).

    GLuint m_fboA, m_fboB;         ///< Framebuffer objects for ping-pong rendering.
    GLuint m_textureA, m_textureB; ///< Textures attached to the FBOs.

    GLuint m_historyFBO;     ///< Framebuffer used to capture the final composite for the next frame.
    GLuint m_historyTex[2];  ///< Double-buffered previous-frame textures (read one, write the other).
    int m_historyIndex;      ///< Index of the texture holding the most recent composite.

    GLuint m_quadVAO; ///< Vertex array object for the screen quad.
    GLuint m_quadVBO; ///< Vertex buffer object for the screen quad.

    std::vector<ProgramPtr> m_fxPrograms; ///< List of shader programs (effects).

public:
    /**
     * @brief Constructor.
     * @param width Width of the render target.
     * @param height Height of the render target.
     */
    PostProcessor(int width, int height);

    /**
     * @brief Destructor.
     */
    virtual ~PostProcessor();

    /**
     * @brief Add a post-processing effect (shader program).
     * @param fxProgram Shared pointer to the shader program.
     */
    void addEffect(ProgramPtr fxProgram);

    /**
     * @brief Remove all post-processing effects.
     */
    void clearEffects();

    /**
     * @brief Initialize the post-processor (framebuffers, textures, quad).
     */
    void initialize();

    /**
     * @brief Resize the post-processor's render target.
     * @param width New width.
     * @param height New height.
     */
    void resize(int width, int height);

    /**
     * @brief Release all resources used by the post-processor.
     */
    void drop();

    /**
     * @brief Factory method to create a shared pointer to a PostProcessor instance.
     * @param width Width of the render target.
     * @param height Height of the render target.
     * @return std::shared_ptr<PostProcessor> New PostProcessor instance.
     */
    static std::shared_ptr<PostProcessor> create(int width, int height);

    /**
     * @brief Initialize the screen quad geometry.
     */
    void initQuad();

    /**
     * @brief Set the current time (for time-based effects).
     * @param time Time value.
     */
    void setTime(float time);

    /**
     * @brief Apply all post-processing effects to the given texture.
     * @param texture OpenGL texture ID to process.
     */
    void apply(GLuint texture);

    /**
     * @brief Get the OpenGL ID of the final output texture.
     * @return GLuint Texture ID.
     */
    GLuint id();
};

/**
 * @typedef PostProcessorPtr
 * @brief Shared pointer type for PostProcessor.
 */
typedef std::shared_ptr<PostProcessor> PostProcessorPtr;

}; // namespace ivf
