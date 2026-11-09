#pragma once

/**
 * @file planar_reflection.h
 * @brief Renders the scene mirrored in a plane into a texture, for water and mirrors.
 */

#include <ivf/composite_node.h>
#include <ivf/framebuffer.h>
#include <ivf/node.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace ivf {

/**
 * @class PlanarReflection
 * @brief Mirror render pass for a reflecting plane such as a lake surface.
 *
 * render() draws the scene from the current camera mirrored in the plane into
 * an offscreen texture. Geometry below the plane is clipped with an oblique
 * near plane, so no shader changes are needed. Nodes added with addHiddenNode()
 * (typically the reflecting surface itself) are not drawn in the pass.
 *
 * A surface samples the reflection at its own screen position:
 * `texture(reflectionTexture, gl_FragCoord.xy / viewportSize)`, see WaterMaterial.
 *
 * With GLFWSceneWindow, register it with addReflection() and it is rendered
 * every frame after the shadow maps and before the main pass:
 * @code
 * auto reflection = PlanarReflection::create({0, 0, 0}, {0, 1, 0});
 * reflection->addHiddenNode(lake);
 * addReflection(reflection);
 * lake->setMaterial(WaterMaterial::create(reflection));
 * @endcode
 */
class PlanarReflection {
private:
    glm::vec3 m_point;
    glm::vec3 m_normal;
    float m_resolutionScale{0.5f};
    float m_clipOffset{0.02f};
    int m_samples{4};
    bool m_enabled{true};

    std::shared_ptr<FrameBuffer> m_frameBuffer;
    int m_width{0};
    int m_height{0};

    std::vector<std::weak_ptr<Node>> m_hiddenNodes;

public:
    /**
     * @brief Constructor.
     * @param point A point on the reflecting plane.
     * @param normal Plane normal, pointing towards the reflected side.
     */
    PlanarReflection(glm::vec3 point = glm::vec3(0.0f), glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f));

    static std::shared_ptr<PlanarReflection> create(glm::vec3 point = glm::vec3(0.0f),
                                                    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f));

    /**
     * @brief Set the reflecting plane.
     * @param point A point on the plane.
     * @param normal Plane normal, pointing towards the reflected side.
     */
    void setPlane(glm::vec3 point, glm::vec3 normal);
    [[nodiscard]] glm::vec3 planePoint() const;
    [[nodiscard]] glm::vec3 planeNormal() const;

    /**
     * @brief Set the reflection texture size relative to the viewport (default 0.5).
     */
    void setResolutionScale(float scale);
    [[nodiscard]] float resolutionScale() const;

    /**
     * @brief Set how far below the plane geometry is still drawn in the reflection.
     *
     * A small positive value avoids gaps where objects meet the surface.
     */
    void setClipOffset(float offset);
    [[nodiscard]] float clipOffset() const;

    /**
     * @brief Set the number of multisampling samples of the reflection (default 4, 0 = off).
     *
     * Anti-aliasing matters more here than on screen, because the reflection is
     * rendered at reduced resolution and magnified.
     */
    void setSamples(int samples);
    [[nodiscard]] int samples() const;

    void setEnabled(bool enabled);
    [[nodiscard]] bool enabled() const;

    /**
     * @brief Exclude a node from the reflection pass, typically the reflecting surface.
     */
    void addHiddenNode(std::shared_ptr<Node> node);
    void clearHiddenNodes();

    /**
     * @brief Render the scene mirrored in the plane, using the current camera.
     *
     * Reads the camera from TransformManager and restores the camera, framebuffer
     * binding, viewport and node visibility afterwards.
     * @param scene Scene to render.
     */
    void render(CompositeNodePtr scene);

    /**
     * @brief Reflection color texture, 0 before the first render().
     *
     * The texture is recreated when the viewport size changes, so fetch it every frame.
     */
    [[nodiscard]] GLuint texture() const;

    /**
     * @brief Reflection matrix that mirrors world space in the plane.
     */
    [[nodiscard]] glm::mat4 reflectionMatrix() const;
};

using PlanarReflectionPtr = std::shared_ptr<PlanarReflection>;

} // namespace ivf
