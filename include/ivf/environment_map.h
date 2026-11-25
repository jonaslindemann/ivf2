#pragma once

/**
 * @file environment_map.h
 * @brief HDR environments for image-based lighting (IBL) and skyboxes.
 */

#include <ivf/cubemap.h>
#include <ivf/glbase.h>
#include <ivf/result.h>

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>

namespace ivf {

/// Texture units the PBR shader reads the environment from.
inline constexpr GLuint IrradianceMapUnit = 12;
inline constexpr GLuint PrefilterMapUnit = 13;
inline constexpr GLuint BrdfLutUnit = 14;

/**
 * @brief A procedural HDR sky: gradient, ground and a sun (EnvironmentMap::loadSky()).
 *
 * Colours are radiance, so values above 1 are bright; the sun is usually far brighter
 * than the sky.
 */
struct SkySettings {
    glm::vec3 zenithColor{0.18f, 0.35f, 0.8f};     ///< Sky straight up.
    glm::vec3 horizonColor{0.85f, 0.9f, 1.0f};     ///< Sky at the horizon.
    glm::vec3 groundColor{0.25f, 0.22f, 0.18f};    ///< Below the horizon.
    glm::vec3 sunDirection{0.5f, 0.45f, -0.6f};    ///< Towards the sun (normalised internally).
    glm::vec3 sunColor{1.0f, 0.93f, 0.8f};         ///< Colour of the sun and its glow.
    float sunIntensity{60.0f};                     ///< Radiance of the sun disc.
    float sunRadiusDeg{2.5f};                      ///< Angular radius of the disc in degrees.
    float sunGlow{0.6f};                           ///< Brightness of the haze around the sun.
    int width{1024};                               ///< Width of the generated equirectangular image.
};

/**
 * @class EnvironmentMap
 * @brief An HDR environment, prepared for image-based lighting.
 *
 * Loads an equirectangular (latitude-longitude) HDR image - a Radiance `.hdr` file, raw
 * float data or a procedural sky - and precomputes on the GPU what the PBR shader needs:
 *
 * - cubemap(): the environment as an HDR cube map with mip levels (also the skybox),
 * - irradiance(): diffuse light from every direction (cosine-weighted convolution),
 * - prefiltered(): reflections for increasing roughness, one roughness per mip level
 *   (GGX importance sampling),
 * - brdfLut(): the split-sum BRDF lookup table.
 *
 * Use it with PBRMaterial::setEnvironment() and as the skybox:
 *
 * @code
 * auto env = ivf::EnvironmentMap::create();
 * if (!env->load("assets/studio.hdr"))   // or env->loadSky(ivf::SkySettings{})
 *     return -1;
 * material->setEnvironment(env);
 * add(ivf::Skybox::create(env->cubemap()));
 * @endcode
 *
 * Loading needs a current OpenGL context (like Texture and Cubemap). `.exr` files are not
 * supported.
 */
class EnvironmentMap : public GLBase {
public:
    EnvironmentMap();
    ~EnvironmentMap() override;

    EnvironmentMap(const EnvironmentMap &) = delete;
    EnvironmentMap &operator=(const EnvironmentMap &) = delete;

    [[nodiscard]] static std::shared_ptr<EnvironmentMap> create();

    /**
     * @brief Load an equirectangular image (Radiance `.hdr`; 8-bit formats work too) and
     *        precompute the lighting maps.
     */
    [[nodiscard]] Result<void> load(const std::string &path);

    /**
     * @brief Use equirectangular RGB float data (@p width x @p height, top row first).
     */
    [[nodiscard]] Result<void> loadEquirectangular(const float *rgb, int width, int height);

    /** @brief Generate a procedural sky (see SkySettings) and precompute the lighting maps. */
    [[nodiscard]] Result<void> loadSky(const SkySettings &settings = {});

    /**
     * @brief Equirectangular RGB float image of a procedural sky (width x width/2, top row
     *        first); what loadSky() uses. Can be saved with writeHdr().
     */
    [[nodiscard]] static std::vector<float> skyImage(const SkySettings &settings);

    /** @brief The environment as an HDR cube map with mip levels (for Skybox). */
    [[nodiscard]] CubemapPtr cubemap() const
    {
        return m_environment;
    }

    /** @brief Diffuse irradiance cube map. */
    [[nodiscard]] CubemapPtr irradiance() const
    {
        return m_irradiance;
    }

    /** @brief Prefiltered specular cube map (mip level = roughness * (levels - 1)). */
    [[nodiscard]] CubemapPtr prefiltered() const
    {
        return m_prefiltered;
    }

    /** @brief Split-sum BRDF lookup table (RG16F, u = N.V, v = roughness). */
    [[nodiscard]] GLuint brdfLut() const
    {
        return m_brdfLut;
    }

    /** @brief True once a load succeeded. */
    [[nodiscard]] bool isValid() const
    {
        return m_valid;
    }

    /**
     * @brief Bind the irradiance, prefiltered and BRDF maps to IrradianceMapUnit,
     *        PrefilterMapUnit and BrdfLutUnit.
     */
    void bind() const;

    /** @brief Size of the environment cube map faces for loaded images (default 512). */
    void setCubemapSize(int size)
    {
        m_cubemapSize = size;
    }

private:
    CubemapPtr m_environment;
    CubemapPtr m_irradiance;
    CubemapPtr m_prefiltered;
    GLuint m_brdfLut{0};
    bool m_valid{false};
    int m_cubemapSize{512};

    Result<void> precompute(GLuint equirectTexture, int cubemapSize);
};

using EnvironmentMapPtr = std::shared_ptr<EnvironmentMap>;

} // namespace ivf
