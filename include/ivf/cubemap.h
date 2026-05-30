#pragma once

/**
 * @file cubemap.h
 * @brief OpenGL cubemap texture for skyboxes and environment reflections.
 *
 * @code
 * auto cm = Cubemap::create();
 * cm->load("right.jpg","left.jpg","top.jpg","bottom.jpg","front.jpg","back.jpg");
 * // or:
 * cm->loadFromDirectory("textures/skybox/"); // loads right/left/top/bottom/front/back.*
 * @endcode
 */

#include <ivf/glbase.h>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <memory>

namespace ivf {

class Cubemap : public GLBase {
public:
    Cubemap();
    ~Cubemap() override;

    static std::shared_ptr<Cubemap> create();

    /**
     * @brief Load 6 face images (right, left, top, bottom, front, back).
     */
    bool load(std::string_view posX, std::string_view negX,
              std::string_view posY, std::string_view negY,
              std::string_view posZ, std::string_view negZ);

    /**
     * @brief Scan a directory for right/left/top/bottom/front/back files
     *        with common extensions (.jpg, .png, .hdr).
     */
    bool loadFromDirectory(std::string_view dir);

    /**
     * @brief Generate a procedural gradient sky cubemap (no file assets needed).
     *
     * Each pixel maps its 3D direction to a smooth gradient between
     * @p topColor (sky zenith), @p horizonColor (equator), and @p bottomColor.
     *
     * @param topColor     RGB color at the top of the sky.
     * @param horizonColor RGB color at the horizon.
     * @param bottomColor  RGB color at the ground.
     * @param size         Texel resolution per face (power-of-two, e.g. 64 or 128).
     */
    bool loadGradient(glm::vec3 topColor    = {0.05f, 0.15f, 0.45f},
                      glm::vec3 horizonColor = {0.45f, 0.65f, 0.85f},
                      glm::vec3 bottomColor  = {0.18f, 0.12f, 0.08f},
                      int size = 64);

    void bind(int unit = 0);
    void unbind();

    [[nodiscard]] GLuint id() const { return m_id; }

private:
    GLuint m_id{0};

    static bool loadFace(GLenum target, std::string_view path);
    static std::string findFile(std::string_view dir, std::string_view stem);
};

using CubemapPtr = std::shared_ptr<Cubemap>;

} // namespace ivf
