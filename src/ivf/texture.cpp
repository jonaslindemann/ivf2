#include <ivf/texture.h>

#include <ivf/shader_manager.h>
#include <ivf/utils.h>

#include <iostream>

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

bool Texture::load(const std::string filename)
{
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);

    cout << "Image channels: " << nrChannels << endl;

    if (data)
    {
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
        std::cout << "Failed to load texture" << std::endl;
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

void ivf::Texture::setIntFormat(GLint format)
{
    m_intFormat = format;
}

void ivf::Texture::setFormat(GLenum format)
{
    m_format = format;
}

void ivf::Texture::setType(GLenum type)
{
    m_type = type;
}

void ivf::Texture::setLevel(GLint level)
{
    m_level = level;
}

void ivf::Texture::setTexUnit(GLint unit)
{
    m_texUnit = unit;
}

void ivf::Texture::setUseLocalBlendMode(bool flag)
{
    m_useLocalBlendMode = flag;
}

void ivf::Texture::setBlendMode(TextureBlendMode blendMode)
{
    m_blendMode = blendMode;
}

void ivf::Texture::setBlendFactor(float blendFactor)
{
    m_blendFactor = blendFactor;
}

void ivf::Texture::setWrapT(GLint wrapT)
{
    m_wrapT = wrapT;
}

void ivf::Texture::setWrapS(GLint wrapS)
{
    m_wrapS = wrapS;
}

void ivf::Texture::setMinFilter(GLint minFilter)
{
    m_minFilter = minFilter;
}

void ivf::Texture::setMagFilter(GLint magFilter)
{
    m_magFilter = magFilter;
}

GLint ivf::Texture::useLocalBlendMode()
{
    return m_useLocalBlendMode;
}

TextureBlendMode ivf::Texture::blendMode()
{
    return m_blendMode;
}

float ivf::Texture::blendFactor()
{
    return m_blendFactor;
}

GLint ivf::Texture::texUnit()
{
    return m_texUnit;
}

GLuint ivf::Texture::id()
{
    return m_id;
}
