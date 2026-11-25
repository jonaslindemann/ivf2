#include <ivf/texture3d.h>

#include <ivf/gl_debug.h>
#include <ivf/logger.h>
#include <ivf/render_context.h>

#include <algorithm>

using namespace ivf;

Texture3D::~Texture3D()
{
    for (auto &gpu : m_gpu)
        release(gpu);
}

std::shared_ptr<Texture3D> Texture3D::create()
{
    return std::make_shared<Texture3D>();
}

void Texture3D::setData(const glm::ivec3 &dimensions, std::span<const float> values, Format format)
{
    const glm::ivec3 dims = glm::max(dimensions, glm::ivec3(0));
    const auto count = static_cast<std::size_t>(dims.x) * static_cast<std::size_t>(dims.y) *
                       static_cast<std::size_t>(dims.z);
    if (values.size() < count)
    {
        logError("Texture3D::setData(): fewer values than dimensions.x * y * z", "Texture3D");
        return;
    }
    m_dimensions = dims;
    m_data.assign(values.begin(), values.begin() + static_cast<std::ptrdiff_t>(count));
    m_format = format;
    ++m_version;
}

glm::ivec3 Texture3D::dimensions() const
{
    return m_dimensions;
}

std::span<const float> Texture3D::data() const
{
    return m_data;
}

std::span<float> Texture3D::data()
{
    return m_data;
}

Texture3D::Format Texture3D::format() const
{
    return m_format;
}

void Texture3D::setLinearFiltering(bool linear)
{
    m_linear = linear;
    ++m_version;
}

bool Texture3D::linearFiltering() const
{
    return m_linear;
}

void Texture3D::update()
{
    ++m_version;
}

GLuint Texture3D::bind(RenderContext &context, int unit)
{
    if (m_data.empty())
        return 0;
    std::erase_if(m_gpu, [](const Gpu &gpu) { return gpu.lifetime.expired(); });
    Gpu *gpu = nullptr;
    for (auto &candidate : m_gpu)
        if (candidate.context == &context)
            gpu = &candidate;
    if (gpu == nullptr)
    {
        m_gpu.push_back(Gpu{&context, context.lifetime(), 0, 0});
        gpu = &m_gpu.back();
    }

    glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
    if (gpu->version != m_version)
        upload(*gpu);
    glBindTexture(GL_TEXTURE_3D, gpu->texture);
    return gpu->texture;
}

void Texture3D::unbind(int unit)
{
    glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
    glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE0);
}

void Texture3D::upload(Gpu &gpu)
{
    if (gpu.texture == 0)
    {
        glGenTextures(1, &gpu.texture);
        glBindTexture(GL_TEXTURE_3D, gpu.texture);
        glLabel(GL_TEXTURE, gpu.texture, "Texture3D");
    }
    glBindTexture(GL_TEXTURE_3D, gpu.texture);

    GLint alignmentWas = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignmentWas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (m_format == Format::R8)
    {
        std::vector<std::uint8_t> bytes(m_data.size());
        for (std::size_t i = 0; i < m_data.size(); ++i)
            bytes[i] = static_cast<std::uint8_t>(std::clamp(m_data[i], 0.0f, 1.0f) * 255.0f + 0.5f);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, m_dimensions.x, m_dimensions.y, m_dimensions.z, 0, GL_RED,
                     GL_UNSIGNED_BYTE, bytes.data());
    }
    else
    {
        const GLint internal = m_format == Format::R32F ? GL_R32F : GL_R16F;
        glTexImage3D(GL_TEXTURE_3D, 0, internal, m_dimensions.x, m_dimensions.y, m_dimensions.z, 0, GL_RED, GL_FLOAT,
                     m_data.data());
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignmentWas);

    const GLint filter = m_linear ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gpu.version = m_version;
}

void Texture3D::release(Gpu &gpu)
{
    if (gpu.lifetime.expired() || gpu.context == nullptr || gpu.texture == 0)
        return;
    const GLuint texture = gpu.texture;
    auto destroy = [texture]() { glDeleteTextures(1, &texture); };
    if (gpu.context->isCurrent())
        destroy();
    else
        gpu.context->deleteLater(destroy);
}
