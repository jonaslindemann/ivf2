#pragma once

/**
 * @file behaviors.h
 * @brief Built-in Behavior implementations for common interactive and creative-coding patterns.
 */

#include <ivf/behavior.h>
#include <ivf/transform_node.h>
#include <ivf/material.h>
#include <ivf/math_utils.h>
#include <ivfmath/spline.h>

#include <glm/gtc/constants.hpp>
#include <functional>
#include <cmath>

namespace ivf {

// ---- SpinBehavior ----------------------------------------------------------

/**
 * @brief Continuously rotates a TransformNode around an axis.
 *
 * @code
 * cube->addBehavior(std::make_shared<SpinBehavior>(glm::vec3(0,1,0), 90.0f));
 * @endcode
 */
class SpinBehavior : public Behavior {
public:
    SpinBehavior(glm::vec3 axis = {0,1,0}, float degreesPerSecond = 45.0f)
        : m_axis(axis), m_speed(degreesPerSecond)
    {}

    void onUpdate(Node* owner, float dt) override
    {
        auto* tn = dynamic_cast<TransformNode*>(owner);
        if (!tn) return;
        m_angle += m_speed * dt;
        tn->setRotAxis(m_axis);
        tn->setRotAngle(m_angle);
    }

private:
    glm::vec3 m_axis;
    float m_speed;
    float m_angle{0.0f};
};

// ---- PulseBehavior ---------------------------------------------------------

/**
 * @brief Oscillates the scale of a TransformNode between minScale and maxScale.
 *
 * @code
 * sphere->addBehavior(std::make_shared<PulseBehavior>(0.8f, 1.2f, 2.0f));
 * @endcode
 */
class PulseBehavior : public Behavior {
public:
    PulseBehavior(float minScale = 0.8f, float maxScale = 1.2f, float frequencyHz = 1.0f)
        : m_min(minScale), m_max(maxScale), m_freq(frequencyHz)
    {}

    void onUpdate(Node* owner, float dt) override
    {
        auto* tn = dynamic_cast<TransformNode*>(owner);
        if (!tn) return;
        m_t += dt;
        float s = map(std::sin(glm::two_pi<float>() * m_freq * m_t), -1.0f, 1.0f, m_min, m_max);
        tn->setScale(glm::vec3(s));
    }

private:
    float m_min, m_max, m_freq;
    float m_t{0.0f};
};

// ---- HoverHighlightBehavior ------------------------------------------------

/**
 * @brief Swaps the node's material when the mouse is over it.
 *
 * @code
 * auto normal = Material::create(); normal->setDiffuseColor({0.5,0.5,0.5,1});
 * auto hover  = Material::create(); hover ->setDiffuseColor({1,0.5,0,1});
 * cube->addBehavior(std::make_shared<HoverHighlightBehavior>(normal, hover));
 * @endcode
 */
class HoverHighlightBehavior : public Behavior {
public:
    HoverHighlightBehavior(MaterialPtr normalMat, MaterialPtr hoverMat)
        : m_normal(normalMat), m_hover(hoverMat)
    {}

    void onStart(Node* owner) override { owner->setMaterial(m_normal); }
    void onUpdate(Node* /*owner*/, float /*dt*/) override {}
    void onMouseEnter(Node* owner) override { owner->setMaterial(m_hover); }
    void onMouseLeave(Node* owner) override { owner->setMaterial(m_normal); }

private:
    MaterialPtr m_normal, m_hover;
};

// ---- ClickCallbackBehavior -------------------------------------------------

/**
 * @brief Fires a callback when the node is clicked.
 *
 * @code
 * btn->addBehavior(std::make_shared<ClickCallbackBehavior>([]{ doSomething(); }));
 * // Or filter by button:
 * btn->addBehavior(std::make_shared<ClickCallbackBehavior>(
 *     []{ doSomething(); }, GLFW_MOUSE_BUTTON_LEFT));
 * @endcode
 */
class ClickCallbackBehavior : public Behavior {
public:
    explicit ClickCallbackBehavior(std::function<void()> callback, int buttonFilter = -1)
        : m_callback(std::move(callback)), m_button(buttonFilter)
    {}

    void onUpdate(Node* /*owner*/, float /*dt*/) override {}
    void onClick(Node* /*owner*/, int button) override
    {
        if (m_button < 0 || button == m_button)
            if (m_callback) m_callback();
    }

private:
    std::function<void()> m_callback;
    int m_button;
};

// ---- FollowSplineBehavior --------------------------------------------------

/**
 * @brief Moves a TransformNode along a Spline at a given speed.
 *
 * @code
 * auto spline = ivfmath::Spline::create();
 * spline->addPoint({0,0,0}); spline->addPoint({5,2,0}); // ...
 * node->addBehavior(std::make_shared<FollowSplineBehavior>(spline, 3.0f));
 * @endcode
 */
class FollowSplineBehavior : public Behavior {
public:
    FollowSplineBehavior(std::shared_ptr<ivfmath::Spline> spline, float speed = 1.0f,
                         bool loop = true)
        : m_spline(std::move(spline)), m_speed(speed), m_loop(loop)
    {}

    void onUpdate(Node* owner, float dt) override
    {
        auto* tn = dynamic_cast<TransformNode*>(owner);
        if (!tn || !m_spline) return;

        m_dist += m_speed * dt;
        float len = m_spline->totalLength();
        if (len <= 0.0f) return;

        if (m_loop) m_dist = std::fmod(m_dist, len);
        else        m_dist = std::min(m_dist, len);

        glm::vec3 p = m_spline->positionByDistance(m_dist);
        tn->setPos(p);
    }

    void setSpeed(float s)  { m_speed = s; }
    void setLoop(bool loop) { m_loop = loop; }

private:
    std::shared_ptr<ivfmath::Spline> m_spline;
    float m_speed;
    bool  m_loop;
    float m_dist{0.0f};
};

// ---- LambdaBehavior --------------------------------------------------------

/**
 * @brief Behavior driven by a lambda — the quickest way to add custom logic.
 *
 * @code
 * node->addBehavior(LambdaBehavior::create([](ivf::Node* n, float dt){
 *     auto* tn = dynamic_cast<ivf::TransformNode*>(n);
 *     if (tn) tn->setPos({sin(t) * 2, 0, 0});
 * }));
 * @endcode
 */
class LambdaBehavior : public Behavior {
public:
    using UpdateFn = std::function<void(Node*, float)>;

    explicit LambdaBehavior(UpdateFn fn) : m_fn(std::move(fn)) {}

    static std::shared_ptr<LambdaBehavior> create(UpdateFn fn)
    {
        return std::make_shared<LambdaBehavior>(std::move(fn));
    }

    void onUpdate(Node* owner, float dt) override { if (m_fn) m_fn(owner, dt); }

private:
    UpdateFn m_fn;
};

} // namespace ivf
