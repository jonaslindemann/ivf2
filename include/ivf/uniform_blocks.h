#pragma once

/**
 * @file uniform_blocks.h
 * @brief Uniform blocks (std140 uniform buffers) shared by all shader programs.
 *
 * Data that is the same for every program in a pass - the camera and the lights - lives
 * in a uniform buffer owned by the render context instead of being uploaded to each
 * program. Program::link() connects a program that declares one of
 * these blocks to its fixed binding point, so shaders only declare the block:
 *
 * @code{.glsl}
 * #version 330 core
 * layout(std140) uniform IvfCamera {
 *     mat4 view;
 *     mat4 projection;
 *     mat4 viewProjection;
 *     vec3 viewPos;
 * };
 * uniform mat4 model;
 * ...
 *     gl_Position = projection * view * model * vec4(aPos, 1.0);
 * @endcode
 *
 * The block has no instance name, so its members are used like plain uniforms. Declare
 * it identically in every stage that uses one of its members (a program cannot have both
 * the block and a plain uniform named `view` or `viewPos`). C++ code can paste
 * IVF_CAMERA_BLOCK_GLSL into a shader string. Programs that declare plain `view`,
 * `projection` and `viewPos` uniforms instead still get them set as before.
 *
 * Lights work the same way with the `IvfLights` block (IVF_LIGHTS_BLOCK_GLSL declares the
 * `PointLight`, `DirLight` and `SpotLight` structs, the arrays `pointLights`,
 * `dirLights`, `spotLights`, `lightSpaceMatrices` and the counts). Shadow map samplers
 * cannot live in a uniform block and stay plain uniforms (`shadowMaps[i]` belongs to
 * `dirLights[i]`).
 */

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <cstdint>

namespace ivf {

/// Binding point of the `IvfCamera` uniform block.
inline constexpr GLuint CameraBlockBinding = 0;

/// Name of the camera uniform block in GLSL.
inline constexpr const char *CameraBlockName = "IvfCamera";

/**
 * @brief Contents of the `IvfCamera` uniform block (std140 layout).
 *
 * Written by TransformManager whenever view, projection or view position change, and by
 * the Renderer at the start of each pass.
 */
struct CameraBlock {
    glm::mat4 view{1.0f};           ///< World to view space.
    glm::mat4 projection{1.0f};     ///< View to clip space.
    glm::mat4 viewProjection{1.0f}; ///< projection * view.
    glm::vec4 viewPos{0.0f};        ///< Camera position in world space (xyz; `vec3 viewPos` in GLSL).
};

static_assert(sizeof(CameraBlock) == 208, "CameraBlock must match the std140 layout of IvfCamera");

/// Binding point of the `IvfLights` uniform block.
inline constexpr GLuint LightsBlockBinding = 1;

/// Name of the lights uniform block in GLSL.
inline constexpr const char *LightsBlockName = "IvfLights";

/// Array sizes of the `IvfLights` block (lights beyond these are ignored by the shaders).
inline constexpr int MaxPointLights = 8;
inline constexpr int MaxDirLights = 4;
inline constexpr int MaxSpotLights = 8;

/// One point light in the `IvfLights` block (std140; GLSL struct `PointLight`).
struct PointLightData {
    glm::vec3 position{0.0f};
    std::int32_t enabled{0};
    glm::vec3 ambientColor{0.0f};
    float constant{1.0f};
    glm::vec3 diffuseColor{0.0f};
    float linear{0.0f};
    glm::vec3 specularColor{0.0f};
    float quadratic{0.0f};
};

/// One directional light in the `IvfLights` block (std140; GLSL struct `DirLight`).
struct DirLightData {
    glm::vec3 direction{0.0f, 0.0f, 1.0f};
    std::int32_t enabled{0};
    glm::vec3 ambientColor{0.0f};
    std::int32_t castShadows{0};
    glm::vec3 diffuseColor{0.0f};
    float shadowStrength{0.0f};
    glm::vec3 specularColor{0.0f};
    float padding{0.0f};
};

/// One spot light in the `IvfLights` block (std140; GLSL struct `SpotLight`).
struct SpotLightData {
    glm::vec3 position{0.0f};
    std::int32_t enabled{0};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
    float cutOff{0.0f}; ///< Cosine of the inner cone angle.
    glm::vec3 ambientColor{0.0f};
    float outerCutOff{0.0f}; ///< Cosine of the outer cone angle.
    glm::vec3 diffuseColor{0.0f};
    float constant{1.0f};
    glm::vec3 specularColor{0.0f};
    float linear{0.0f};
    float quadratic{0.0f};
    float padding[3]{};
};

/**
 * @brief Contents of the `IvfLights` uniform block (std140 layout).
 *
 * Written by LightManager::apply() when it changed. Counts are clamped to the array sizes.
 */
struct LightsBlock {
    PointLightData pointLights[MaxPointLights];
    DirLightData dirLights[MaxDirLights];
    SpotLightData spotLights[MaxSpotLights];
    glm::mat4 lightSpaceMatrices[MaxDirLights]{}; ///< Shadow map matrix of dirLights[i].
    std::int32_t pointLightCount{0};
    std::int32_t dirLightCount{0};
    std::int32_t spotLightCount{0};
    std::int32_t padding{0};
};

static_assert(sizeof(PointLightData) == 64, "PointLightData must match the std140 layout of PointLight");
static_assert(sizeof(DirLightData) == 64, "DirLightData must match the std140 layout of DirLight");
static_assert(sizeof(SpotLightData) == 96, "SpotLightData must match the std140 layout of SpotLight");
static_assert(sizeof(LightsBlock) == 1808, "LightsBlock must match the std140 layout of IvfLights");

} // namespace ivf

/// GLSL declaration of the camera uniform block, for pasting into shader source strings.
#define IVF_CAMERA_BLOCK_GLSL                                                                                          \
    "layout(std140) uniform IvfCamera {\n"                                                                             \
    "    mat4 view;\n"                                                                                                 \
    "    mat4 projection;\n"                                                                                           \
    "    mat4 viewProjection;\n"                                                                                       \
    "    vec3 viewPos;\n"                                                                                              \
    "};\n"

/// GLSL declaration of the light structs and the lights uniform block, for pasting into
/// shader source strings (fragment stage). Defines IVF_MAX_POINT_LIGHTS, IVF_MAX_DIR_LIGHTS
/// and IVF_MAX_SPOT_LIGHTS (the same sizes as MaxPointLights etc.).
#define IVF_LIGHTS_BLOCK_GLSL \
    "#define IVF_MAX_POINT_LIGHTS 8\n" \
    "#define IVF_MAX_DIR_LIGHTS 4\n" \
    "#define IVF_MAX_SPOT_LIGHTS 8\n" \
    "struct PointLight {\n" \
    "    vec3 position; bool enabled;\n" \
    "    vec3 ambientColor; float constant;\n" \
    "    vec3 diffuseColor; float linear;\n" \
    "    vec3 specularColor; float quadratic;\n" \
    "};\n" \
    "struct DirLight {\n" \
    "    vec3 direction; bool enabled;\n" \
    "    vec3 ambientColor; bool castShadows;\n" \
    "    vec3 diffuseColor; float shadowStrength;\n" \
    "    vec3 specularColor;\n" \
    "};\n" \
    "struct SpotLight {\n" \
    "    vec3 position; bool enabled;\n" \
    "    vec3 direction; float cutOff;\n" \
    "    vec3 ambientColor; float outerCutOff;\n" \
    "    vec3 diffuseColor; float constant;\n" \
    "    vec3 specularColor; float linear;\n" \
    "    float quadratic;\n" \
    "};\n" \
    "layout(std140) uniform IvfLights {\n" \
    "    PointLight pointLights[IVF_MAX_POINT_LIGHTS];\n" \
    "    DirLight dirLights[IVF_MAX_DIR_LIGHTS];\n" \
    "    SpotLight spotLights[IVF_MAX_SPOT_LIGHTS];\n" \
    "    mat4 lightSpaceMatrices[IVF_MAX_DIR_LIGHTS];\n" \
    "    int pointLightCount;\n" \
    "    int dirLightCount;\n" \
    "    int spotLightCount;\n" \
    "};\n"
