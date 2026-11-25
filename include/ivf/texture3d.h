#pragma once

/**
 * @file texture3d.h
 * @brief 3D textures of single-channel data (volumes), uploaded per render context.
 */

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace ivf {

class RenderContext;

/**
 * @class Texture3D
 * @brief A 3D texture with one float channel, e.g. the values of a volume.
 *
 * The data is kept on the CPU and uploaded to each render context that binds the texture,
 * when it is first bound and again after update(), like Mesh. So textures can be created
 * without an OpenGL context and used in several windows.
 *
 * @code
 * auto tex = ivf::Texture3D::create();
 * tex->setData({64, 64, 64}, values);          // x varies fastest
 * tex->bind(*renderContext(), 3);             // texture unit 3
 * @endcode
 */
class Texture3D {
public:
    /** @brief Internal format on the GPU. */
    enum class Format {
        R8,   ///< 8 bits, values 0..1 (others are clamped).
        R16F, ///< Half floats (default): any range, about 3 significant digits.
        R32F  ///< Full floats.
    };

    Texture3D() = default;
    ~Texture3D();

    Texture3D(const Texture3D &) = delete;
    Texture3D &operator=(const Texture3D &) = delete;

    [[nodiscard]] static std::shared_ptr<Texture3D> create();

    /** @brief The texels, x fastest then y then z (dimensions.x * y * z values). */
    void setData(const glm::ivec3 &dimensions, std::span<const float> values, Format format = Format::R16F);

    [[nodiscard]] glm::ivec3 dimensions() const;
    [[nodiscard]] std::span<const float> data() const;
    [[nodiscard]] Format format() const;

    /** @brief Linear (default) or nearest filtering. */
    void setLinearFiltering(bool linear);
    [[nodiscard]] bool linearFiltering() const;

    /** @brief Upload the data again before the next bind (after changing it in place). */
    void update();

    /** @brief Writable texels; call update() after changing them. */
    [[nodiscard]] std::span<float> data();

    /**
     * @brief Bind to texture unit @p unit in @p context (uploading first if needed).
     * @return The OpenGL texture name (0 if there is no data).
     */
    GLuint bind(RenderContext &context, int unit);

    /** @brief Unbind GL_TEXTURE_3D from texture unit @p unit. */
    static void unbind(int unit);

private:
    struct Gpu {
        RenderContext *context{nullptr};
        std::weak_ptr<const void> lifetime;
        GLuint texture{0};
        std::uint64_t version{0};
    };
    void upload(Gpu &gpu);
    static void release(Gpu &gpu);

    glm::ivec3 m_dimensions{0};
    std::vector<float> m_data;
    Format m_format{Format::R16F};
    bool m_linear{true};
    std::uint64_t m_version{0};
    std::vector<Gpu> m_gpu;
};

using Texture3DPtr = std::shared_ptr<Texture3D>;

} // namespace ivf
