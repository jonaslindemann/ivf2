#include <ivf/planar_reflection.h>

#include <ivf/transform_manager.h>

#include <algorithm>
#include <cmath>

using namespace ivf;

namespace {

float sign(float v)
{
    return v > 0.0f ? 1.0f : (v < 0.0f ? -1.0f : 0.0f);
}

// Replace the near plane of an OpenGL perspective projection with an arbitrary
// view-space clip plane (Lengyel, "Oblique View Frustum Depth Projection and
// Clipping"). Points with dot(clipPlane, p) < 0 are clipped.
glm::mat4 obliqueProjection(glm::mat4 projection, const glm::vec4 &clipPlane)
{
    glm::vec4 q;
    q.x = (sign(clipPlane.x) + projection[2][0]) / projection[0][0];
    q.y = (sign(clipPlane.y) + projection[2][1]) / projection[1][1];
    q.z = -1.0f;
    q.w = (1.0f + projection[2][2]) / projection[3][2];

    glm::vec4 c = clipPlane * (2.0f / glm::dot(clipPlane, q));

    projection[0][2] = c.x;
    projection[1][2] = c.y;
    projection[2][2] = c.z + 1.0f;
    projection[3][2] = c.w;
    return projection;
}

} // namespace

PlanarReflection::PlanarReflection(glm::vec3 point, glm::vec3 normal)
{
    setPlane(point, normal);
}

std::shared_ptr<PlanarReflection> PlanarReflection::create(glm::vec3 point, glm::vec3 normal)
{
    return std::make_shared<PlanarReflection>(point, normal);
}

void PlanarReflection::setPlane(glm::vec3 point, glm::vec3 normal)
{
    m_point = point;
    m_normal = glm::length(normal) > 0.0f ? glm::normalize(normal) : glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 PlanarReflection::planePoint() const
{
    return m_point;
}

glm::vec3 PlanarReflection::planeNormal() const
{
    return m_normal;
}

void PlanarReflection::setResolutionScale(float scale)
{
    m_resolutionScale = std::clamp(scale, 0.1f, 1.0f);
}

float PlanarReflection::resolutionScale() const
{
    return m_resolutionScale;
}

void PlanarReflection::setClipOffset(float offset)
{
    m_clipOffset = offset;
}

float PlanarReflection::clipOffset() const
{
    return m_clipOffset;
}

void PlanarReflection::setSamples(int samples)
{
    samples = std::max(samples, 0);
    if (samples == m_samples)
        return;
    m_samples = samples;
    m_frameBuffer.reset(); // recreated with the new setting on the next render()
}

int PlanarReflection::samples() const
{
    return m_samples;
}

void PlanarReflection::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool PlanarReflection::enabled() const
{
    return m_enabled;
}

void PlanarReflection::addHiddenNode(std::shared_ptr<Node> node)
{
    m_hiddenNodes.push_back(node);
}

void PlanarReflection::clearHiddenNodes()
{
    m_hiddenNodes.clear();
}

GLuint PlanarReflection::texture() const
{
    return m_frameBuffer ? m_frameBuffer->colorTexture() : 0;
}

glm::mat4 PlanarReflection::reflectionMatrix() const
{
    // Householder reflection in the plane n.x + d = 0
    const glm::vec3 n = m_normal;
    const float d = -glm::dot(n, m_point);

    glm::mat4 r(1.0f);
    for (int col = 0; col < 3; ++col)
        for (int row = 0; row < 3; ++row)
            r[col][row] -= 2.0f * n[row] * n[col];
    r[3] = glm::vec4(-2.0f * d * n, 1.0f);
    return r;
}

void PlanarReflection::render(CompositeNodePtr scene)
{
    if (!m_enabled || !scene)
        return;

    // Save the state this pass changes
    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);
    GLint previousFramebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);

    const int width = std::max(1, int(previousViewport[2] * m_resolutionScale));
    const int height = std::max(1, int(previousViewport[3] * m_resolutionScale));
    if (!m_frameBuffer)
    {
        m_frameBuffer = FrameBuffer::create(width, height);
        if (m_samples > 0)
        {
            m_frameBuffer->setSamples(m_samples);
            m_frameBuffer->setMultisample(true);
        }
        else
        {
            m_frameBuffer->initialize();
        }
    }
    else if (width != m_width || height != m_height)
    {
        m_frameBuffer->resize(width, height);
    }
    m_width = width;
    m_height = height;

    auto xfm = TransformManager::instance();
    const glm::mat4 view = xfm->viewMatrix();
    const glm::mat4 projection = xfm->projectionMatrix();
    const glm::vec3 viewPos = xfm->viewPos();

    // Mirror the camera in the plane
    const glm::mat4 reflection = reflectionMatrix();
    const glm::mat4 reflectedView = view * reflection;
    const glm::vec3 reflectedViewPos = glm::vec3(reflection * glm::vec4(viewPos, 1.0f));

    // Clip everything below the plane (keep dot(plane, p) >= 0) via an oblique near plane
    const glm::vec4 worldPlane(m_normal, -glm::dot(m_normal, m_point) + m_clipOffset);
    const glm::vec4 viewPlane = glm::transpose(glm::inverse(reflectedView)) * worldPlane;

    // Hide the reflecting surface itself
    std::vector<std::pair<std::shared_ptr<Node>, bool>> hidden;
    for (const auto &weak : m_hiddenNodes)
        if (auto node = weak.lock())
        {
            hidden.emplace_back(node, node->visible());
            node->setVisible(false);
        }

    m_frameBuffer->begin();
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    xfm->viewMatrix() = reflectedView;
    xfm->projectionMatrix() = obliqueProjection(projection, viewPlane);
    xfm->setViewPos(reflectedViewPos);
    xfm->refreshForProgram();

    scene->draw();

    // Restore
    xfm->viewMatrix() = view;
    xfm->projectionMatrix() = projection;
    xfm->setViewPos(viewPos);
    xfm->refreshForProgram();

    for (auto &[node, wasVisible] : hidden)
        node->setVisible(wasVisible);

    m_frameBuffer->end(); // resolves the multisampled buffer into the texture
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer));
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
}
