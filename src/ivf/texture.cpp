#include <ivf/texture.h>

#include <ivf/shader_manager.h>
#include <ivf/utils.h>
#include <ivf/logger.h>

#include <algorithm>
#include <iostream>
#include <string_view>

// GL_EXT_texture_filter_anisotropic — defined in core 4.6 and as an extension before that.
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

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

        GLenum fmt;
        GLint  ifmt;
        switch (nrChannels) {
            case 1:  fmt = GL_RED;  ifmt = GL_RED;   break;
            case 2:  fmt = GL_RG;   ifmt = GL_RG;    break;
            case 3:  fmt = GL_RGB;  ifmt = GL_RGB;   break;
            default: fmt = GL_RGBA; ifmt = GL_RGBA;  break;
        }

        this->bind();
        GL_ERR_BEGIN;
        glTexImage2D(GL_TEXTURE_2D, m_level, ifmt, width, height, 0, fmt, m_type, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GL_ERR_END("Texture::load()");
        this->unbind();
    }
    else
    {
        logErrorfc("Texture", "Failed to load texture from file: {}", filename);
        stbi_image_free(data);
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

void Texture::setAnisotropicFiltering(float level)
{
    m_anisotropy = level;
    if (m_id && m_anisotropy > 0.0f) {
        float maxAniso = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        std::min(m_anisotropy, maxAniso));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

bool Texture::loadHDR(std::string_view filename)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    float* data = stbi_loadf(std::string(filename).c_str(), &width, &height, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(false);

    if (!data) {
        logErrorfc("Texture", "Failed to load HDR: {}", filename);
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    logInfofc("Texture", "Loaded HDR texture: {} ({}x{})", filename, width, height);
    return true;
}
