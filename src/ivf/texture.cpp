#include <ivf/texture.h>

#include <ivf/shader_manager.h>
#include <ivf/utils.h>
#include <ivf/logger.h>

#include <iostream>
#include <string_view>

using namespace ivf;
using namespace std;

Texture::Texture()
    : m_id(0), m_wrapT(GL_REPEAT), m_wrapS(GL_REPEAT), m_minFilter(GL_LINEAR_MIPMAP_LINEAR), m_magFilter(GL_LINEAR),
      m_intFormat(GL_RGBA), m_level(0), m_format(GL_RGBA), m_type(GL_UNSIGNED_BYTE), m_texUnit(0),
      m_blendMode(TextureBlendMode::Normal), m_blendFactor(1.0), m_useLocalBlendMode(false)
{
    glGenTextures(1, &m_id);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

std::shared_ptr<Texture> Texture::create()
{
    return std::make_shared<Texture>();
}

void Texture::bind()
{
    GL_ERR_BEGIN;
    glActiveTexture(GL_TEXTURE0 + m_texUnit);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);
    if (m_useLocalBlendMode)
    {
        TextureManager::instance()->setTextureBlendMode(m_blendMode);
        TextureManager::instance()->setBlendFactor(m_blendFactor);
    }
    GL_ERR_END("Texture::bind()");
}

bool Texture::load(const std::string_view filename)
{
    logInfofc("Texture", "Loading texture from file: {}", filename);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(std::string(filename).c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        logInfofc("Texture", "Loaded texture: {} ({}x{}, {} channels)", filename, width, height, nrChannels);
        this->bind();
        GL_ERR_BEGIN;
        glTexImage2D(GL_TEXTURE_2D, m_level, m_format, width, height, 0, m_intFormat, m_type, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GL_ERR_END("Texture::load()");
        this->unbind();
    }
    else
    {
        stbi_image_free(data);
        logErrorfc("Texture", "Failed to load texture from file: {}", filename);
        return false;
    }
    stbi_image_free(data);

    return true;
}

void Texture::unbind()
{
    if (m_useLocalBlendMode)
        TextureManager::instance()->restoreState();
    GL_ERR(glBindTexture(GL_TEXTURE_2D, 0));
}
