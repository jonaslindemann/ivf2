#include <ivfphysics/interaction.h>

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <cmath>

using namespace ivfphysics;

// ---------------------------------------------------------------------------
// DragController
// ---------------------------------------------------------------------------

DragController::DragController(const PhysicsWorldPtr &world, const DragSettings &settings)
    : m_world(world), m_settings(settings)
{
    m_callback = world->addStepCallback([this](double h) { applySpring(h); });
}

std::shared_ptr<DragController> DragController::create(const PhysicsWorldPtr &world, const DragSettings &settings)
{
    if (!world)
        return nullptr;
    return std::shared_ptr<DragController>(new DragController(world, settings));
}

DragController::~DragController()
{
    release();
    if (auto world = m_world.lock())
        world->removeStepCallback(m_callback);
}

bool DragController::grab(const ivf::Ray &ray, const RaycastOptions &options)
{
    auto world = m_world.lock();
    if (!world)
        return false;
    const auto hit = world->raycast(ray, options);
    if (!hit || hit.body->type() != BodyType::Dynamic)
        return false;
    if (!grab(hit.body, hit.point, ray.origin))
        return false;
    m_rayOrigin = ray.origin;
    m_rayDirection = glm::normalize(ray.direction);
    return true;
}

bool DragController::grab(const RigidBodyPtr &body, const glm::vec3 &point, const glm::vec3 &rayOrigin)
{
    release();
    if (!body || !body->isValid() || body->type() != BodyType::Dynamic)
        return false;
    m_body = body;
    m_localAnchor = glm::inverse(body->rotation()) * (point - body->position());
    m_target = point;
    m_rayOrigin = rayOrigin;
    m_distance = glm::length(point - rayOrigin);
    m_rayDirection = m_distance > 0.0f ? (point - rayOrigin) / m_distance : glm::vec3(0.0f, 0.0f, -1.0f);
    m_savedAngularDamping = body->angularDamping();
    body->setDamping(body->linearDamping(), std::max(m_savedAngularDamping, m_settings.angularDamping));
    body->wakeUp();
    return true;
}

void DragController::drag(const ivf::Ray &ray)
{
    const float length = glm::length(ray.direction);
    if (!(length > 0.0f))
        return;
    m_rayOrigin = ray.origin;
    m_rayDirection = ray.direction / length;
    m_target = m_rayOrigin + m_rayDirection * m_distance;
}

void DragController::setTarget(const glm::vec3 &target)
{
    m_target = target;
}

void DragController::setDistance(float distance)
{
    m_distance = std::max(distance, 0.0f);
    m_target = m_rayOrigin + m_rayDirection * m_distance;
}

void DragController::release()
{
    if (m_body && m_body->isValid())
        m_body->setDamping(m_body->linearDamping(), m_savedAngularDamping);
    m_body.reset();
}

bool DragController::isDragging() const
{
    return m_body != nullptr;
}

RigidBodyPtr DragController::body() const
{
    return m_body;
}

glm::vec3 DragController::target() const
{
    return m_target;
}

glm::vec3 DragController::anchor() const
{
    if (!m_body || !m_body->isValid())
        return m_target;
    return m_body->position() + m_body->rotation() * m_localAnchor;
}

float DragController::distance() const
{
    return m_distance;
}

const DragSettings &DragController::settings() const
{
    return m_settings;
}

void DragController::setSettings(const DragSettings &settings)
{
    m_settings = settings;
}

void DragController::applySpring(double /*h*/)
{
    if (!m_body)
        return;
    if (!m_body->isValid() || m_body->type() != BodyType::Dynamic)
    {
        m_body.reset(); // removed or changed while held
        return;
    }

    // Mass-normalised damped spring (acceleration k x - c v), so every body follows alike
    const float omega = 2.0f * glm::pi<float>() * std::max(m_settings.frequency, 0.0f);
    const float k = omega * omega;
    const float c = 2.0f * std::max(m_settings.dampingRatio, 0.0f) * omega;
    const glm::vec3 point = anchor();
    glm::vec3 acceleration = k * (m_target - point) - c * m_body->velocityAt(point);
    const float a = glm::length(acceleration);
    if (m_settings.maxAcceleration > 0.0f && a > m_settings.maxAcceleration)
        acceleration *= m_settings.maxAcceleration / a;
    m_body->applyForceAt(m_body->mass() * acceleration, point);
}

// ---------------------------------------------------------------------------
// Throwing
// ---------------------------------------------------------------------------

void ivfphysics::throwBody(const RigidBodyPtr &body, const ivf::Ray &ray, float speed, const glm::vec3 &spin)
{
    if (!body || !body->isValid())
        return;
    const float length = glm::length(ray.direction);
    const glm::vec3 direction = length > 0.0f ? ray.direction / length : glm::vec3(0.0f, 0.0f, -1.0f);
    body->setPosition(ray.origin);
    body->setLinearVelocity(direction * speed);
    body->setAngularVelocity(spin);
    body->wakeUp();
}
