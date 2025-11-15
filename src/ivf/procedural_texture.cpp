#include <ivf/procedural_texture.h>
#include <ivf/utils.h>
#include <ivf/logger.h>
#include <algorithm>

using namespace ivf;

// ============================================================================
// ProceduralTexture Base Class
// ============================================================================

ProceduralTexture::ProceduralTexture()
    : Texture()
{
}

ProceduralTexture::~ProceduralTexture()
{
}

void ProceduralTexture::setSize(int width, int height)
{
    if (width != m_width || height != m_height) {
        m_width = width;
        m_height = height;
        m_needsRegeneration = true;
    }
}

void ProceduralTexture::setChannels(int channels)
{
    if (channels == 3 || channels == 4) {
        if (channels != m_channels) {
            m_channels = channels;
            m_needsRegeneration = true;
        }
    }
}

void ProceduralTexture::setPixel(int x, int y, const glm::vec4 &color)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return;

    int index = (y * m_width + x) * m_channels;
    
    m_data[index + 0] = static_cast<unsigned char>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
    m_data[index + 1] = static_cast<unsigned char>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
    m_data[index + 2] = static_cast<unsigned char>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
    
    if (m_channels == 4) {
        m_data[index + 3] = static_cast<unsigned char>(std::clamp(color.a, 0.0f, 1.0f) * 255.0f);
    }
}

void ProceduralTexture::regenerate()
{
    logInfofc("ProceduralTexture", "Regenerating texture: {}x{}, {} channels", m_width, m_height, m_channels);
    
    // Allocate data buffer
    m_data.resize(m_width * m_height * m_channels);
    
    // Generate the texture
    generate();
    
    // Upload to GPU
    upload();
    
    m_needsRegeneration = false;
}

void ProceduralTexture::upload()
{
    if (m_data.empty()) {
        logErrorfc("ProceduralTexture", "No data to upload");
        return;
    }

    GLenum format = (m_channels == 4) ? GL_RGBA : GL_RGB;
    GLenum internalFormat = (m_channels == 4) ? GL_RGBA : GL_RGB;
    
    // Set the texture format in the base class
    this->setFormat(format);
    this->setIntFormat(internalFormat);
    
    GL_ERR_BEGIN;
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->id());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data (note: using same convention as Texture::load())
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, internalFormat, GL_UNSIGNED_BYTE, m_data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    GL_ERR_END("ProceduralTexture::upload()");
    
    logInfofc("ProceduralTexture", "Uploaded texture to GPU: {}x{}", m_width, m_height);
}

void ProceduralTexture::setNeedsRegeneration(bool flag)
{
    m_needsRegeneration = flag;
}

// ============================================================================
// CheckerboardTexture
// ============================================================================

CheckerboardTexture::CheckerboardTexture()
    : ProceduralTexture()
{
}

std::shared_ptr<CheckerboardTexture> CheckerboardTexture::create()
{
    return std::make_shared<CheckerboardTexture>();
}

void CheckerboardTexture::setCheckerSize(int size)
{
    if (size != m_checkerSize) {
        m_checkerSize = size;
        m_needsRegeneration = true;
    }
}

void CheckerboardTexture::setColors(const glm::vec4 &color1, const glm::vec4 &color2)
{
    m_color1 = color1;
    m_color2 = color2;
    m_needsRegeneration = true;
}

glm::vec4 CheckerboardTexture::getPixel(float u, float v)
{
    float checker = ProcUtils::checkerboard(u * m_width, v * m_height, static_cast<float>(m_checkerSize));
    return (checker > 0.5f) ? m_color1 : m_color2;
}

void CheckerboardTexture::generate()
{
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            float u = static_cast<float>(x) / m_width;
            float v = static_cast<float>(y) / m_height;
            setPixel(x, y, getPixel(u, v));
        }
    }
}

// ============================================================================
// PerlinNoiseTexture
// ============================================================================

PerlinNoiseTexture::PerlinNoiseTexture()
    : ProceduralTexture()
{
}

std::shared_ptr<PerlinNoiseTexture> PerlinNoiseTexture::create()
{
    return std::make_shared<PerlinNoiseTexture>();
}

void PerlinNoiseTexture::setScale(float scale)
{
    if (scale != m_scale) {
        m_scale = scale;
        m_needsRegeneration = true;
    }
}

void PerlinNoiseTexture::setOctaves(int octaves)
{
    if (octaves != m_octaves) {
        m_octaves = octaves;
        m_needsRegeneration = true;
    }
}

void PerlinNoiseTexture::setPersistence(float persistence)
{
    if (persistence != m_persistence) {
        m_persistence = persistence;
        m_needsRegeneration = true;
    }
}

void PerlinNoiseTexture::setLacunarity(float lacunarity)
{
    if (lacunarity != m_lacunarity) {
        m_lacunarity = lacunarity;
        m_needsRegeneration = true;
    }
}

void PerlinNoiseTexture::setColorRange(const glm::vec4 &colorLow, const glm::vec4 &colorHigh)
{
    m_colorLow = colorLow;
    m_colorHigh = colorHigh;
    m_needsRegeneration = true;
}

glm::vec4 PerlinNoiseTexture::getPixel(float u, float v)
{
    float noise = ProcUtils::fbm(u * m_scale, v * m_scale, m_octaves, m_persistence, m_lacunarity);
    float t = (noise + 1.0f) * 0.5f; // Map from [-1, 1] to [0, 1]
    return ProcUtils::mixColors(m_colorLow, m_colorHigh, t);
}

void PerlinNoiseTexture::generate()
{
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            float u = static_cast<float>(x) / m_width;
            float v = static_cast<float>(y) / m_height;
            setPixel(x, y, getPixel(u, v));
        }
    }
}

// ============================================================================
// GradientTexture
// ============================================================================

GradientTexture::GradientTexture()
    : ProceduralTexture()
{
}

std::shared_ptr<GradientTexture> GradientTexture::create()
{
    return std::make_shared<GradientTexture>();
}

void GradientTexture::setType(GradientType type)
{
    if (type != m_type) {
        m_type = type;
        m_needsRegeneration = true;
    }
}

void GradientTexture::setColors(const glm::vec4 &colorStart, const glm::vec4 &colorEnd)
{
    m_colorStart = colorStart;
    m_colorEnd = colorEnd;
    m_needsRegeneration = true;
}

void GradientTexture::setAngle(float angle)
{
    if (angle != m_angle) {
        m_angle = angle;
        m_needsRegeneration = true;
    }
}

void GradientTexture::setCenter(const glm::vec2 &center)
{
    m_center = center;
    m_needsRegeneration = true;
}

glm::vec4 GradientTexture::getPixel(float u, float v)
{
    float t = 0.0f;
    
    if (m_type == GradientType::Linear) {
        t = ProcUtils::gradient(u, v, m_angle);
    } else {
        t = ProcUtils::radialGradient(u, v, m_center.x, m_center.y);
    }
    
    return ProcUtils::mixColors(m_colorStart, m_colorEnd, t);
}

void GradientTexture::generate()
{
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            float u = static_cast<float>(x) / m_width;
            float v = static_cast<float>(y) / m_height;
            setPixel(x, y, getPixel(u, v));
        }
    }
}

// ============================================================================
// MarbleTexture
// ============================================================================

MarbleTexture::MarbleTexture()
    : ProceduralTexture()
{
}

std::shared_ptr<MarbleTexture> MarbleTexture::create()
{
    return std::make_shared<MarbleTexture>();
}

void MarbleTexture::setScale(float scale)
{
    if (scale != m_scale) {
        m_scale = scale;
        m_needsRegeneration = true;
    }
}

void MarbleTexture::setOctaves(int octaves)
{
    if (octaves != m_octaves) {
        m_octaves = octaves;
        m_needsRegeneration = true;
    }
}

void MarbleTexture::setColors(const glm::vec4 &color1, const glm::vec4 &color2)
{
    m_color1 = color1;
    m_color2 = color2;
    m_needsRegeneration = true;
}

glm::vec4 MarbleTexture::getPixel(float u, float v)
{
    float marble = ProcUtils::marble(u, v, m_scale, m_octaves);
    return ProcUtils::mixColors(m_color1, m_color2, marble);
}

void MarbleTexture::generate()
{
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            float u = static_cast<float>(x) / m_width;
            float v = static_cast<float>(y) / m_height;
            setPixel(x, y, getPixel(u, v));
        }
    }
}

// ============================================================================
// WoodTexture
// ============================================================================

WoodTexture::WoodTexture()
    : ProceduralTexture()
{
}

std::shared_ptr<WoodTexture> WoodTexture::create()
{
    return std::make_shared<WoodTexture>();
}

void WoodTexture::setScale(float scale)
{
    if (scale != m_scale) {
        m_scale = scale;
        m_needsRegeneration = true;
    }
}

void WoodTexture::setRings(float rings)
{
    if (rings != m_rings) {
        m_rings = rings;
        m_needsRegeneration = true;
    }
}

void WoodTexture::setColors(const glm::vec4 &color1, const glm::vec4 &color2)
{
    m_color1 = color1;
    m_color2 = color2;
    m_needsRegeneration = true;
}

glm::vec4 WoodTexture::getPixel(float u, float v)
{
    // Center the coordinates
    float x = (u - 0.5f) * 2.0f;
    float y = (v - 0.5f) * 2.0f;
    
    float wood = ProcUtils::wood(x, y, m_scale, m_rings);
    return ProcUtils::mixColors(m_color1, m_color2, wood);
}

void WoodTexture::generate()
{
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            float u = static_cast<float>(x) / m_width;
            float v = static_cast<float>(y) / m_height;
            setPixel(x, y, getPixel(u, v));
        }
    }
}
