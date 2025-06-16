#pragma once

#include <ivf/glbase.h>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class ShadowMap
 * @brief Manages an OpenGL depth texture and framebuffer for shadow mapping.
 *
 * The ShadowMap class encapsulates the creation and management of a framebuffer object (FBO)
 * and depth texture used for shadow mapping in real-time rendering. It provides methods to
 * initialize, resize, bind/unbind the shadow map, and set or retrieve the light space matrix
 * used for shadow projection.
 */
class ShadowMap : public GLBase {
private:
    GLuint m_fbo{0};              ///< Framebuffer object for shadow rendering.
    GLuint m_depthTexture{0};     ///< Depth texture for storing shadow map.
    int m_width{1024};            ///< Width of the shadow map (default 1024).
    int m_height{1024};           ///< Height of the shadow map (default 1024).
    glm::mat4 m_lightSpaceMatrix; ///< Light space transformation matrix.

public:
    /**
     * @brief Constructor.
     * @param width Width of the shadow map.
     * @param height Height of the shadow map.
     */
    ShadowMap(int width = 1024, int height = 1024);

    /**
     * @brief Destructor.
     */
    virtual ~ShadowMap();

    /**
     * @brief Factory method to create a shared pointer to a ShadowMap instance.
     * @param width Width of the shadow map.
     * @param height Height of the shadow map.
     * @return std::shared_ptr<ShadowMap> New ShadowMap instance.
     */
    static std::shared_ptr<ShadowMap> create(int width = 1024, int height = 1024);

    /**
     * @brief Initialize the shadow map resources (FBO and depth texture).
     */
    void initialize();

    /**
     * @brief Resize the shadow map.
     * @param width New width.
     * @param height New height.
     */
    void resize(int width, int height);

    /**
     * @brief Bind the shadow map for rendering.
     */
    void bind();

    /**
     * @brief Unbind the shadow map (restore previous framebuffer).
     */
    void unbind();

    /**
     * @brief Get the OpenGL depth texture ID.
     * @return GLuint Depth texture ID.
     */
    GLuint depthTexture() const;

    /**
     * @brief Set the light space transformation matrix.
     * @param matrix Light space matrix.
     */
    void setLightSpaceMatrix(const glm::mat4 &matrix);

    /**
     * @brief Get the light space transformation matrix.
     * @return glm::mat4 Light space matrix.
     */
    glm::mat4 lightSpaceMatrix() const;
};

/**
 * @typedef ShadowMapPtr
 * @brief Shared pointer type for ShadowMap.
 */
typedef std::shared_ptr<ShadowMap> ShadowMapPtr;

} // namespace ivf
