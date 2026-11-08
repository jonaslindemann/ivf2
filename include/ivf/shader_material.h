#pragma once

/**
 * @file shader_material.h
 * @brief Material that renders a node with a custom GLSL program.
 */

#include <ivf/material.h>
#include <ivf/program.h>
#include <ivf/texture.h>

#include <glm/glm.hpp>

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace ivf {

/**
 * @class ShaderMaterial
 * @brief Material that renders its node with a custom shader program and uniforms.
 *
 * Assign it to any node with Node::setMaterial(). When the node is drawn, apply()
 * makes the program current, re-targets the transform, light and selection managers
 * to it and uploads the uniforms and textures. unapply() restores the program that
 * was current before.
 *
 * @section Shader requirements
 * - Vertex attributes at the stock locations (meshes bind these indices):
 *   @code
 *   layout (location = 0) in vec3 aPos;
 *   layout (location = 1) in vec4 aColor;
 *   layout (location = 2) in vec2 aTex;
 *   layout (location = 3) in vec3 aNormal;
 *   @endcode
 * - Provided automatically: `uniform mat4 model, view, projection;` and `uniform vec3 viewPos;`
 * - The stock lighting uniforms (light arrays, `material.*`) are uploaded only when
 *   setUseLighting(true) is set, so unlit shaders do not need to declare them.
 *
 * Uniforms a shader does not declare are ignored, so one set of global uniforms
 * (e.g. time or music envelopes, see setGlobalUniform()) can feed every shader.
 *
 * Child nodes render with this program too, unless they have a program-switching
 * material of their own. As with PBRMaterial, the program is also used when the
 * node is drawn into shadow maps.
 *
 * @code
 * auto glow = ShaderMaterial::createFromFiles("shaders/glow.vert", "shaders/glow.frag", "glow", true);
 * glow->setUniform("glowColor", glm::vec3(0.4f, 1.0f, 0.8f));
 * sphere->setMaterial(glow);
 *
 * // every frame
 * ShaderMaterial::setGlobalUniform("time", float(elapsedTime()));
 * @endcode
 */
class ShaderMaterial : public Material {
public:
    using UniformValue = std::variant<float, int, bool, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

private:
    struct TextureBinding {
        TexturePtr texture;
        unsigned int textureId{0};
        int unit{0};
    };

    ProgramPtr m_program;
    std::map<std::string, UniformValue> m_uniforms;
    std::map<std::string, TextureBinding> m_textures;
    std::vector<ProgramPtr> m_previousPrograms; ///< Stack, so nested use of one material restores correctly.

    static std::map<std::string, UniformValue> m_globalUniforms;

    void refreshManagers();
    void bindTextures();
    void unbindTextures();

public:
    /**
     * @brief Constructor.
     * @param program Shader program to render with. Lighting is disabled by default.
     */
    ShaderMaterial(ProgramPtr program = nullptr);

    /**
     * @brief Factory method.
     * @param program Shader program to render with.
     */
    static std::shared_ptr<ShaderMaterial> create(ProgramPtr program = nullptr);

    /**
     * @brief Load a program from vertex and fragment shader files.
     *
     * The program is registered with ShaderManager under @p name.
     *
     * @param vertPath Vertex shader path.
     * @param fragPath Fragment shader path.
     * @param name     Program name.
     * @param watch    If true, the files are reloaded on change via ShaderWatcher
     *                 (GLFWSceneWindow updates the watcher every frame).
     * @return The material, or nullptr if the shaders failed to compile or link.
     */
    static std::shared_ptr<ShaderMaterial> createFromFiles(const std::string &vertPath, const std::string &fragPath,
                                                           const std::string &name, bool watch = false);

    /**
     * @brief Create a program from vertex and fragment shader source strings.
     *
     * The program is registered with ShaderManager under @p name.
     * @return The material, or nullptr if the shaders failed to compile or link.
     */
    static std::shared_ptr<ShaderMaterial> createFromStrings(const std::string &vertSource,
                                                             const std::string &fragSource, const std::string &name);

    void setProgram(ProgramPtr program);
    [[nodiscard]] ProgramPtr program() const;

    /**
     * @brief Set a uniform uploaded every time the material is applied.
     *
     * Overrides a global uniform with the same name.
     * @param name  Uniform name in the shader.
     * @param value Value. Use explicit types, e.g. 1.0f for float, 1 for int.
     */
    void setUniform(const std::string &name, UniformValue value);

    /**
     * @brief Get a uniform value previously set with setUniform().
     * @return Pointer to the value, or nullptr if not set.
     */
    [[nodiscard]] const UniformValue *uniform(const std::string &name) const;

    void removeUniform(const std::string &name);
    void clearUniforms();

    /**
     * @brief Bind a texture to a sampler uniform.
     * @param samplerName Sampler uniform name in the shader.
     * @param texture     Texture to bind.
     * @param unit        Texture unit to bind it to.
     */
    void setTexture(const std::string &samplerName, TexturePtr texture, int unit);

    /**
     * @brief Bind a raw OpenGL 2D texture (e.g. a framebuffer color attachment) to a sampler uniform.
     * @param samplerName Sampler uniform name in the shader.
     * @param textureId   OpenGL texture name.
     * @param unit        Texture unit to bind it to.
     */
    void setTexture(const std::string &samplerName, unsigned int textureId, int unit);

    void removeTexture(const std::string &samplerName);
    void clearTextures();

    /**
     * @brief Set a uniform uploaded to every ShaderMaterial, e.g. time or audio levels.
     *
     * Material uniforms with the same name take precedence.
     */
    static void setGlobalUniform(const std::string &name, UniformValue value);
    static void removeGlobalUniform(const std::string &name);
    static void clearGlobalUniforms();

    void apply() override;
    void unapply() override;
};

using ShaderMaterialPtr = std::shared_ptr<ShaderMaterial>;

} // namespace ivf
