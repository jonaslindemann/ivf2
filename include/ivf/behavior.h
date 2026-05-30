#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace ivf {

class Node;
class TransformNode;

/**
 * @brief Abstract base for tick-able behaviors attached to scene nodes.
 *
 * Derive from Behavior and override onUpdate() (and optionally the mouse/click
 * hooks) to create reusable interactive components that attach directly to nodes.
 *
 * Usage:
 * @code
 * class BounceBehavior : public ivf::Behavior {
 *     void onUpdate(ivf::Node* node, float dt) override {
 *         auto* tn = dynamic_cast<ivf::TransformNode*>(node);
 *         if (tn) tn->setPos({sin(t) * 2, 0, 0});
 *         t += dt;
 *     }
 *     float t = 0;
 * };
 * sphere->addBehavior(std::make_shared<BounceBehavior>());
 * @endcode
 */
class Behavior {
public:
    virtual ~Behavior() = default;

    /** Called once the first time the behavior's owner is updated. */
    virtual void onStart(Node* /*owner*/) {}

    /** Called every frame. dt is the frame delta time in seconds. */
    virtual void onUpdate(Node* owner, float dt) = 0;

    /** Called when the mouse cursor enters the node's screen area. */
    virtual void onMouseEnter(Node* /*owner*/) {}

    /** Called when the mouse cursor leaves the node's screen area. */
    virtual void onMouseLeave(Node* /*owner*/) {}

    /** Called when the node is clicked. button: GLFW_MOUSE_BUTTON_*. */
    virtual void onClick(Node* /*owner*/, int /*button*/) {}

    [[nodiscard]] bool started() const { return m_started; }
    void markStarted() { m_started = true; }

private:
    bool m_started{false};
};

using BehaviorPtr = std::shared_ptr<Behavior>;

// ---- BehaviorHost ----------------------------------------------------------

/**
 * @brief Mixin that gives any Node the ability to hold and tick behaviors.
 *
 * Node inherits from this class. Call updateBehaviors(dt) each frame and
 * the notify* methods when mouse events arrive.
 */
class BehaviorHost {
private:
    std::vector<BehaviorPtr> m_behaviors;

public:
    virtual ~BehaviorHost() = default;

    void addBehavior(BehaviorPtr behavior) { m_behaviors.push_back(std::move(behavior)); }

    void removeBehavior(BehaviorPtr behavior)
    {
        m_behaviors.erase(
            std::remove(m_behaviors.begin(), m_behaviors.end(), behavior),
            m_behaviors.end());
    }

    void clearBehaviors() { m_behaviors.clear(); }

    [[nodiscard]] bool hasBehaviors() const { return !m_behaviors.empty(); }

    void updateBehaviors(Node* owner, float dt)
    {
        for (auto& b : m_behaviors) {
            if (!b->started()) { b->onStart(owner); b->markStarted(); }
            b->onUpdate(owner, dt);
        }
    }

    void notifyMouseEnter(Node* owner)
    {
        for (auto& b : m_behaviors) b->onMouseEnter(owner);
    }

    void notifyMouseLeave(Node* owner)
    {
        for (auto& b : m_behaviors) b->onMouseLeave(owner);
    }

    void notifyClick(Node* owner, int button)
    {
        for (auto& b : m_behaviors) b->onClick(owner, button);
    }
};

} // namespace ivf
