#include <ivf/blend_shader_material.h>

#include <ivf/logger.h>
#include <ivf/shader_manager.h>
#include <ivf/shader_watcher.h>

#include <glad/glad.h>

using namespace ivf;

namespace {

std::shared_ptr<BlendShaderMaterial> checkedMaterial(ProgramPtr program, const std::string &name, BlendMode mode)
{
    if (ShaderManager::instance()->compileLinkErrors())
    {
        logError("Shader program '" + name + "' failed to compile or link.", "BlendShaderMaterial");
        return nullptr;
    }
    return BlendShaderMaterial::create(program, mode);
}

} // namespace

BlendShaderMaterial::BlendShaderMaterial(ProgramPtr program, BlendMode mode) : ShaderMaterial(program), m_mode(mode)
{
}

std::shared_ptr<BlendShaderMaterial> BlendShaderMaterial::create(ProgramPtr program, BlendMode mode)
{
    return std::make_shared<BlendShaderMaterial>(program, mode);
}

std::shared_ptr<BlendShaderMaterial> BlendShaderMaterial::createFromFiles(const std::string &vertPath,
                                                                          const std::string &fragPath,
                                                                          const std::string &name, BlendMode mode,
                                                                          bool watch)
{
    auto program = ShaderManager::instance()->loadProgramFromFiles(vertPath, fragPath, name, false);
    auto material = checkedMaterial(program, name, mode);
    if (material && watch)
        ShaderWatcher::instance()->watch(program, vertPath, fragPath);
    return material;
}

std::shared_ptr<BlendShaderMaterial> BlendShaderMaterial::createFromStrings(const std::string &vertSource,
                                                                            const std::string &fragSource,
                                                                            const std::string &name, BlendMode mode)
{
    auto program = ShaderManager::instance()->loadProgramFromStrings(vertSource, fragSource, name, false);
    return checkedMaterial(program, name, mode);
}

void BlendShaderMaterial::setBlendMode(BlendMode mode)
{
    m_mode = mode;
}

BlendMode BlendShaderMaterial::blendMode() const
{
    return m_mode;
}

void BlendShaderMaterial::apply()
{
    ShaderMaterial::apply();

    m_blendWasEnabled = glIsEnabled(GL_BLEND) == GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &m_depthMaskWas);
    glGetIntegerv(GL_BLEND_SRC_RGB, &m_srcRgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &m_dstRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &m_srcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &m_dstAlpha);

    if (m_mode == BlendMode::Background)
        glDisable(GL_BLEND);
    else
        glEnable(GL_BLEND);

    if (m_mode == BlendMode::Additive)
        glBlendFunc(GL_ONE, GL_ONE);
    else
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);
}

void BlendShaderMaterial::unapply()
{
    glDepthMask(m_depthMaskWas);
    glBlendFuncSeparate(m_srcRgb, m_dstRgb, m_srcAlpha, m_dstAlpha);
    if (m_blendWasEnabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    ShaderMaterial::unapply();
}
