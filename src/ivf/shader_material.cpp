#include <ivf/shader_material.h>

#include <ivf/light_manager.h>
#include <ivf/logger.h>
#include <ivf/selection_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/shader_watcher.h>
#include <ivf/transform_manager.h>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <type_traits>

using namespace ivf;

std::map<std::string, ShaderMaterial::UniformValue> ShaderMaterial::m_globalUniforms;

namespace {

void uploadUniform(Program &program, const std::string &name, const ShaderMaterial::UniformValue &value)
{
    GLint loc = program.uniformLoc(name);
    if (loc < 0)
        return;

    std::visit(
        [loc](const auto &v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, float>)
                glUniform1f(loc, v);
            else if constexpr (std::is_same_v<T, int>)
                glUniform1i(loc, v);
            else if constexpr (std::is_same_v<T, bool>)
                glUniform1i(loc, v ? 1 : 0);
            else if constexpr (std::is_same_v<T, glm::vec2>)
                glUniform2fv(loc, 1, glm::value_ptr(v));
            else if constexpr (std::is_same_v<T, glm::vec3>)
                glUniform3fv(loc, 1, glm::value_ptr(v));
            else if constexpr (std::is_same_v<T, glm::vec4>)
                glUniform4fv(loc, 1, glm::value_ptr(v));
            else if constexpr (std::is_same_v<T, glm::mat3>)
                glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v));
            else if constexpr (std::is_same_v<T, glm::mat4>)
                glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));
        },
        value);
}

std::shared_ptr<ShaderMaterial> checkedMaterial(ProgramPtr program, const std::string &name)
{
    if (ShaderManager::instance()->compileLinkErrors())
    {
        logError("Shader program '" + name + "' failed to compile or link.", "ShaderMaterial");
        return nullptr;
    }
    return ShaderMaterial::create(program);
}

} // namespace

ShaderMaterial::ShaderMaterial(ProgramPtr program) : m_program(program)
{
    setUseLighting(false);
}

std::shared_ptr<ShaderMaterial> ShaderMaterial::create(ProgramPtr program)
{
    return std::make_shared<ShaderMaterial>(program);
}

std::shared_ptr<ShaderMaterial> ShaderMaterial::createFromFiles(const std::string &vertPath,
                                                                const std::string &fragPath, const std::string &name,
                                                                bool watch)
{
    auto program = ShaderManager::instance()->loadProgramFromFiles(vertPath, fragPath, name, false);
    auto material = checkedMaterial(program, name);
    if (material && watch)
        ShaderWatcher::instance()->watch(program, vertPath, fragPath);
    return material;
}

std::shared_ptr<ShaderMaterial> ShaderMaterial::createFromStrings(const std::string &vertSource,
                                                                  const std::string &fragSource,
                                                                  const std::string &name)
{
    auto program = ShaderManager::instance()->loadProgramFromStrings(vertSource, fragSource, name, false);
    return checkedMaterial(program, name);
}

void ShaderMaterial::setProgram(ProgramPtr program)
{
    m_program = program;
}

ProgramPtr ShaderMaterial::program() const
{
    return m_program;
}

void ShaderMaterial::setUniform(const std::string &name, UniformValue value)
{
    m_uniforms[name] = value;
}

const ShaderMaterial::UniformValue *ShaderMaterial::uniform(const std::string &name) const
{
    auto it = m_uniforms.find(name);
    return it != m_uniforms.end() ? &it->second : nullptr;
}

void ShaderMaterial::removeUniform(const std::string &name)
{
    m_uniforms.erase(name);
}

void ShaderMaterial::clearUniforms()
{
    m_uniforms.clear();
}

void ShaderMaterial::setTexture(const std::string &samplerName, TexturePtr texture, int unit)
{
    m_textures[samplerName] = TextureBinding{texture, 0, unit};
}

void ShaderMaterial::setTexture(const std::string &samplerName, unsigned int textureId, int unit)
{
    m_textures[samplerName] = TextureBinding{nullptr, textureId, unit};
}

void ShaderMaterial::removeTexture(const std::string &samplerName)
{
    m_textures.erase(samplerName);
}

void ShaderMaterial::clearTextures()
{
    m_textures.clear();
}

void ShaderMaterial::setGlobalUniform(const std::string &name, UniformValue value)
{
    m_globalUniforms[name] = value;
}

void ShaderMaterial::removeGlobalUniform(const std::string &name)
{
    m_globalUniforms.erase(name);
}

void ShaderMaterial::clearGlobalUniforms()
{
    m_globalUniforms.clear();
}

void ShaderMaterial::apply()
{
    if (!m_program)
    {
        Material::apply();
        return;
    }

    auto shaderMgr = ShaderManager::instance();
    m_previousPrograms.push_back(shaderMgr->currentProgram());
    shaderMgr->setCurrentProgram(m_program);

    refreshManagers();

    // Stock material uniforms (material.*, useLighting) for shaders that declare them
    Material::apply();

    if (useLighting())
        LightManager::instance()->apply();

    for (const auto &[name, value] : m_globalUniforms)
        if (m_uniforms.find(name) == m_uniforms.end())
            uploadUniform(*m_program, name, value);

    for (const auto &[name, value] : m_uniforms)
        uploadUniform(*m_program, name, value);

    bindTextures();
}

void ShaderMaterial::unapply()
{
    if (m_previousPrograms.empty())
        return;

    unbindTextures();

    auto previous = m_previousPrograms.back();
    m_previousPrograms.pop_back();

    if (previous)
    {
        ShaderManager::instance()->setCurrentProgram(previous);
        refreshManagers();
    }
}

void ShaderMaterial::refreshManagers()
{
    TransformManager::instance()->refreshForProgram();
    LightManager::instance()->refreshForProgram();
    SelectionManager::instance()->refreshForProgram();
}

void ShaderMaterial::bindTextures()
{
    for (const auto &[name, binding] : m_textures)
    {
        if (binding.texture)
        {
            binding.texture->setTexUnit(binding.unit);
            binding.texture->bind();
        }
        else
        {
            glActiveTexture(GL_TEXTURE0 + binding.unit);
            glBindTexture(GL_TEXTURE_2D, binding.textureId);
        }
        m_program->uniformInt(name, binding.unit);
    }
    glActiveTexture(GL_TEXTURE0);
}

void ShaderMaterial::unbindTextures()
{
    for (const auto &[name, binding] : m_textures)
    {
        glActiveTexture(GL_TEXTURE0 + binding.unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}
