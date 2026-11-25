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
#include <ivf/result.h>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <memory>

namespace ivf {

class Cubemap : public GLBase {
public:
    Cubemap();
    ~Cubemap() override;

    [[nodiscard]] static std::shared_ptr<Cubemap> create();

    /**
     * @brief Load 6 face images (right, left, top, bottom, front, back).
     * @return Success, or the error of the first face that failed (also logged).
     */
    Result<void> load(std::string_view posX, std::string_view negX,
              std::string_view posY, std::string_view negY,
              std::string_view posZ, std::string_view negZ);

    /**
     * @brief Scan a directory for right/left/top/bottom/front/back files
     *        with common extensions (.jpg, .png, .hdr).
     */
    Result<void> loadFromDirectory(std::string_view dir);

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

    /**
     * @brief Allocate empty faces of @p size x @p size pixels, for rendering into.
     *
     * Used by EnvironmentMap for its HDR cube maps.
     * @param size Edge length of a face in pixels.
     * @param internalFormat Sized format, e.g. GL_RGBA16F (RGB float formats are not
     *        guaranteed to be renderable).
     * @param levels Mip levels to allocate (1: none); filtering is trilinear if more than one.
     */
    void allocate(int size, GLenum internalFormat, int levels = 1);

    /** @brief Edge length of a face of an allocated cube map (0 for loaded images). */
    [[nodiscard]] int size() const
    {
        return m_size;
    }

    /** @brief Mip levels of an allocated cube map. */
    [[nodiscard]] int levels() const
    {
        return m_levels;
    }

    void bind(int unit = 0);
    void unbind();

    [[nodiscard]] GLuint id() const { return m_id; }

private:
    GLuint m_id{0};
    int m_size{0};   ///< See size().
    int m_levels{1}; ///< See levels().

    static Result<void> loadFace(GLenum target, std::string_view path);
    static std::string findFile(std::string_view dir, std::string_view stem);
};

using CubemapPtr = std::shared_ptr<Cubemap>;

} // namespace ivf
