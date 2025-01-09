#include <ivf/texture_manager.h>

#include <ivf/shader_manager.h>

#include <strstream>

using namespace ivf;

TextureManager *TextureManager::m_instance = 0;

TextureManager::TextureManager()
    : m_textureBlendMode(TextureBlendMode::Normal), m_blendFactor(1.0), m_useTexture(true), m_textureBlendModeId(-1),
      m_blendFactorId(-1), m_useTextureId(-1)
{
    m_textureBlendModeId = ShaderManager::instance()->currentProgram()->uniformLoc("blendMode");
    m_blendFactorId = ShaderManager::instance()->currentProgram()->uniformLoc("blendFactor");
    m_useTextureId = ShaderManager::instance()->currentProgram()->uniformLoc("useTexture");
}

void ivf::TextureManager::setTextureBlendMode(TextureBlendMode blendMode)
{
    m_textureBlendMode = blendMode;
    ShaderManager::instance()->currentProgram()->uniformInt(m_textureBlendModeId, (GLint)m_textureBlendMode);
}

void ivf::TextureManager::setBlendFactor(float blendFactor)
{
    m_blendFactor = blendFactor;
    ShaderManager::instance()->currentProgram()->uniformFloat(m_blendFactorId, m_blendFactor);
}

void TextureManager::setUseTexture(bool flag)
{
    m_useTexture = flag;
    ShaderManager::instance()->currentProgram()->uniformBool(m_useTextureId, m_useTexture);
}

TextureBlendMode ivf::TextureManager::textureBlendMode()
{
    return m_textureBlendMode;
}

GLfloat ivf::TextureManager::blendFactor()
{
    return m_blendFactor;
}

GLboolean ivf::TextureManager::useTexture()
{
    return m_useTexture;
}

void ivf::TextureManager::apply()
{
    ShaderManager::instance()->currentProgram()->uniformInt(m_textureBlendModeId, (GLint)m_textureBlendMode);
    ShaderManager::instance()->currentProgram()->uniformFloat(m_blendFactorId, m_blendFactor);
    ShaderManager::instance()->currentProgram()->uniformBool(m_useTextureId, m_useTexture);
}

void TextureManager::saveState()
{
    m_savedTextureBlendMode = m_textureBlendMode;
    m_savedBlendFactor = m_blendFactor;
    m_savedUseTexture = m_useTexture;
}

void TextureManager::restoreState()
{
    m_textureBlendMode = m_savedTextureBlendMode;
    m_blendFactor = m_savedBlendFactor;
    m_useTexture = m_savedUseTexture;
}
