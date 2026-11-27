#include <ivf/texture.h>
#include <ivf/gl_debug.h>

#include <ivf/image_io.h>

#include <ivf/shader_manager.h>
#include <ivf/utils.h>
#include <ivf/logger.h>

#include <filesystem>
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
      m_intFormat(GL_RGBA), m_level(0), m_texUnit(0), m_format(GL_RGBA), m_type(GL_UNSIGNED_BYTE),
      m_useLocalBlendMode(false), m_blendMode(TextureBlendMode::Normal), m_blendFactor(1.0)
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

namespace {

// Error for an image that could not be decoded: missing file or unsupported data.
std::unexpected<Error> imageError(std::string_view filename)
{
    const std::string name(filename);
    if (!std::filesystem::exists(std::filesystem::path(name)))
        return makeError(ErrorCode::FileNotFound, "Could not load texture '" + name + "': file not found", "Texture");
    return makeError(ErrorCode::UnsupportedFormat,
                     "Could not load texture '" + name + "': " + ivf::imageFailureReason(), "Texture");
}

} // namespace

Result<std::shared_ptr<Texture>> Texture::fromFile(std::string_view filename)
{
    auto texture = Texture::create();
    if (auto loaded = texture->load(filename); !loaded)
        return std::unexpected(loaded.error());
    return texture;
}

Result<void> Texture::load(const std::string_view filename)
{
    logInfofc("Texture", "Loading texture from file: {}", filename);

    int width, height, nrChannels;
    unsigned char *data = ivf::loadImage(std::string(filename).c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        logInfofc("Texture", "Loaded texture: {} ({}x{}, {} channels)", filename, width, height, nrChannels);
        setImage(width, height, nrChannels, data, std::filesystem::path(filename).filename().string());
        m_filename = std::string(filename);
    }
    else
    {
        return imageError(filename);
    }
    ivf::freeImage(data);

    return {};
}

Result<void> Texture::loadFromMemory(const unsigned char *bytes, std::size_t size, std::string_view name)
{
    int width = 0, height = 0, nrChannels = 0;
    unsigned char *data =
        ivf::loadImageFromMemory(bytes, static_cast<int>(size), &width, &height, &nrChannels, 0);
    if (!data)
        return makeError(ErrorCode::UnsupportedFormat, std::string("Could not decode image: ") + std::string(name),
                         "Texture");
    setImage(width, height, nrChannels, data, name);
    ivf::freeImage(data);
    return {};
}

void Texture::setImage(int width, int height, int channels, const unsigned char *pixels, std::string_view name)
{
    GLenum fmt;
    GLint ifmt;
    switch (channels)
    {
    // One and two channels with a sized internal format: OpenGL ES (WebGL) has no unsized
    // GL_RED/GL_RG (desktop GL chooses the same 8-bit formats for them)
    case 1:
        fmt = GL_RED;
        ifmt = m_type == GL_UNSIGNED_BYTE ? GL_R8 : GL_RED;
        break;
    case 2:
        fmt = GL_RG;
        ifmt = m_type == GL_UNSIGNED_BYTE ? GL_RG8 : GL_RG;
        break;
    case 3:
        fmt = GL_RGB;
        ifmt = GL_RGB;
        break;
    default:
        fmt = GL_RGBA;
        ifmt = GL_RGBA;
        break;
    }

    // Rows of 1-3 channel images are not always 4-byte aligned
    GLint alignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    this->bind();
    GL_ERR_BEGIN;
    glTexImage2D(GL_TEXTURE_2D, m_level, ifmt, width, height, 0, fmt, m_type, pixels);
    if (!name.empty())
        glLabel(GL_TEXTURE, m_id, std::string(name));
    glGenerateMipmap(GL_TEXTURE_2D);
    GL_ERR_END("Texture::setImage()");
    this->unbind();

    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
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

Result<void> Texture::loadHDR(std::string_view filename)
{
    ivf::setImageFlipVerticallyOnLoad(true);
    int width, height, nrChannels;
    float* data = ivf::loadImageF(std::string(filename).c_str(), &width, &height, &nrChannels, 0);
    ivf::setImageFlipVerticallyOnLoad(false);

    if (!data)
        return imageError(filename);

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    ivf::freeImage(data);
    logInfofc("Texture", "Loaded HDR texture: {} ({}x{})", filename, width, height);
    return {};
}
