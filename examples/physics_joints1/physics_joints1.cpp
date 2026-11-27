/**
 * @file physics_joints1.cpp
 * @brief Joints and compound bodies with ivfphysics.
 * @author Jonas Lindemann
 * @example physics_joints1.cpp
 * @ingroup physics_examples
 *
 *  - a hanging bridge: planks joined by hinges, its ends hinged to the world
 *  - a pendulum chain of ball and socket joints
 *  - a windmill: a compound cross on a hinge driven by a motor
 *  - a piston: a slider with limits and a motor that reverses at the ends
 *  - a welded tower: fixed joints that break when hit hard (throw a ball with Space)
 *  - a table: one body with a compound shape (top and four legs)
 *  Drag bodies with the left mouse button; L toggles the debug view, R resets (mends the welds).
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/logger.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <ivfphysics/ivfphysics.h>

#include <array>
#include <cmath>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;
using namespace ivfphysics;

namespace {

MaterialPtr makeMaterial(const glm::vec4 &color)
{
    auto material = Material::create();
    material->setDiffuseColor(color);
    material->setSpecularColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
    material->setShininess(30.0f);
    return material;
}

} // namespace

class JointsWindow : public GLFWSceneWindow {
private:
    PhysicsWorldPtr m_world;
    DragControllerPtr m_drag;
    JointPtr m_piston;
    MaterialPtr m_wood, m_metal, m_stone, m_red, m_ball;
    std::vector<RigidBodyPtr> m_thrown;
    DebugViewOptions m_debugOptions;
    bool m_debugView{true};
    int m_brokenWelds{0};

public:
    JointsWindow(int width, int height, const std::string &title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<JointsWindow> create(int width, int height, const std::string &title)
    {
        return std::make_shared<JointsWindow>(width, height, title);
    }

    /** A box node (half extents) at @p pos, rotated @p angle radians about z, with a dynamic body. */
    RigidBodyPtr addBox(const glm::vec3 &half, const glm::vec3 &pos, const MaterialPtr &material, float mass,
                        float angle = 0.0f)
    {
        auto node = Box::create(half);
        node->setPos(pos);
        node->setRotation(glm::angleAxis(angle, glm::vec3(0.0f, 0.0f, 1.0f)));
        node->setMaterial(material);
        this->add(node);
        return m_world->addBody(node, Shape::box(half), {.mass = mass, .friction = 0.6f});
    }

    /** Planks along a sagging curve, hinged to each other and, at the ends, to the world. */
    void buildBridge()
    {
        constexpr int planks = 12;
        constexpr float halfSpan = 3.3f;
        constexpr float height = 2.6f;
        constexpr float sag = 0.6f;
        std::array<glm::vec3, planks + 1> points;
        for (int i = 0; i <= planks; ++i)
        {
            const float x = -halfSpan + 2.0f * halfSpan * static_cast<float>(i) / planks;
            const float t = x / halfSpan;
            points[static_cast<std::size_t>(i)] = {x, height - sag * (1.0f - t * t), 0.0f};
        }

        // Posts for the eye (static, no joints)
        for (const float x : {-halfSpan - 0.15f, halfSpan + 0.15f})
        {
            auto post = Box::create({0.12f, 0.5f * height + 0.15f, 0.7f});
            post->setPos({x, 0.5f * height + 0.15f, 0.0f});
            post->setMaterial(m_stone);
            this->add(post);
            m_world->addBody(post, Shape::box({0.12f, 0.5f * height + 0.15f, 0.7f}), {.type = BodyType::Static});
        }

        RigidBodyPtr previous;
        const glm::vec3 axis(0.0f, 0.0f, 1.0f);
        for (int i = 0; i < planks; ++i)
        {
            const glm::vec3 a = points[static_cast<std::size_t>(i)];
            const glm::vec3 b = points[static_cast<std::size_t>(i) + 1];
            const glm::vec3 d = b - a;
            const float halfLength = 0.5f * glm::length(d) - 0.02f; // small gaps: neighbours do not jam
            auto plank = addBox({halfLength, 0.05f, 0.6f}, 0.5f * (a + b), m_wood, 2.0f, std::atan2(d.y, d.x));
            if (previous)
                m_world->addHingeJoint(previous, plank, a, axis);
            else
                m_world->addHingeJoint(plank, nullptr, a, axis);
            previous = plank;
        }
        m_world->addHingeJoint(previous, nullptr, points.back(), axis);

        // A few crates dropped onto it
        for (int i = 0; i < 3; ++i)
            addBox(glm::vec3(0.25f), {-1.2f + 1.2f * static_cast<float>(i), 4.0f + 0.6f * static_cast<float>(i), 0.0f},
                   m_red, 3.0f);
    }

    /** Spheres joined by ball and socket joints, starting horizontally from a pivot. */
    void buildChain()
    {
        const glm::vec3 pivot(7.0f, 5.5f, 0.0f);
        auto hook = Sphere::create(0.12);
        hook->setPos(pivot);
        hook->setMaterial(m_metal);
        this->add(hook);

        RigidBodyPtr previous;
        for (int i = 1; i <= 6; ++i)
        {
            auto link = Sphere::create(0.2);
            link->setPos(pivot + glm::vec3(-0.5f * static_cast<float>(i), 0.0f, 0.0f));
            link->setMaterial(m_metal);
            this->add(link);
            auto body = m_world->addBody(link, Shape::sphere(0.2f), {.mass = 1.0f, .canSleep = false});
            const glm::vec3 joint = pivot + glm::vec3(-0.5f * static_cast<float>(i - 1), 0.0f, 0.0f);
            if (previous)
                m_world->addBallSocketJoint(previous, body, joint);
            else
                m_world->addBallSocketJoint(body, nullptr, joint);
            previous = body;
        }
    }

    /** A cross (one body, two box shapes) turned by a hinge motor. */
    void buildWindmill()
    {
        const glm::vec3 hub(-7.0f, 3.5f, 0.0f);
        auto tower = Box::create({0.2f, 0.5f * hub.y, 0.2f});
        tower->setPos({hub.x, 0.5f * hub.y, -0.35f});
        tower->setMaterial(m_stone);
        this->add(tower);
        m_world->addBody(tower, Shape::box({0.2f, 0.5f * hub.y, 0.2f}), {.type = BodyType::Static});

        auto cross = CompositeNode::create();
        const glm::vec3 arm(1.6f, 0.12f, 0.08f);
        auto bladeX = Box::create(arm);
        auto bladeY = Box::create({arm.y, arm.x, arm.z});
        bladeX->setMaterial(m_wood);
        bladeY->setMaterial(m_wood);
        cross->add(bladeX);
        cross->add(bladeY);
        cross->setPos(hub);
        this->add(cross);

        auto shape = Shape::compound({Shape::box(arm), Shape::box({arm.y, arm.x, arm.z})});
        auto rotor = m_world->addBody(cross, shape, {.mass = 4.0f, .angularDamping = 0.0f, .canSleep = false});
        auto axle = m_world->addHingeJoint(rotor, nullptr, hub, {0.0f, 0.0f, 1.0f});
        axle->setMotor(1.2f, 80.0f);
    }

    /** A block on a horizontal slider that pushes crates back and forth. */
    void buildPiston()
    {
        const glm::vec3 start(-1.0f, 0.3f, 4.0f);
        auto ram = addBox({0.3f, 0.3f, 0.6f}, start, m_metal, 20.0f);
        ram->setCanSleep(false);
        m_piston = m_world->addSliderJoint(ram, nullptr, start, {1.0f, 0.0f, 0.0f});
        m_piston->setLimits(0.0f, 3.0f);
        m_piston->setMotor(1.5f, 400.0f);
        for (int i = 0; i < 3; ++i)
            addBox(glm::vec3(0.2f), {0.2f + 0.45f * static_cast<float>(i), 0.2f, 4.0f}, m_red, 1.0f);
    }

    /** Four blocks welded on top of each other and to the ground; hard hits break the welds. */
    void buildTower()
    {
        const glm::vec3 base(5.0f, 0.0f, 3.5f);
        RigidBodyPtr below;
        for (int i = 0; i < 4; ++i)
        {
            const glm::vec3 pos = base + glm::vec3(0.0f, 0.35f + 0.7f * static_cast<float>(i), 0.0f);
            auto block = addBox({0.5f, 0.35f, 0.5f}, pos, i % 2 ? m_red : m_stone, 4.0f); // the welds show
            const glm::vec3 seam = pos - glm::vec3(0.0f, 0.35f, 0.0f);
            const JointDesc weld{.breakForce = 400.0f, .breakTorque = 150.0f};
            if (below)
                m_world->addFixedJoint(below, block, seam, weld);
            else
                m_world->addFixedJoint(block, nullptr, seam, weld);
            below = block;
        }
    }

    /** One body with a compound shape: a top and four legs. */
    void buildTable()
    {
        auto table = CompositeNode::create();
        auto top = Box::create({1.0f, 0.05f, 0.6f});
        top->setPos({0.0f, 0.75f, 0.0f});
        top->setMaterial(m_wood);
        table->add(top);
        auto topShape = Shape::box({1.0f, 0.05f, 0.6f});
        topShape->setOffset({0.0f, 0.75f, 0.0f});
        std::vector<ShapePtr> parts{topShape};
        for (const float x : {-0.9f, 0.9f})
            for (const float z : {-0.5f, 0.5f})
            {
                auto leg = Box::create({0.05f, 0.35f, 0.05f});
                leg->setPos({x, 0.35f, z});
                leg->setMaterial(m_wood);
                table->add(leg);
                auto legShape = Shape::box({0.05f, 0.35f, 0.05f});
                legShape->setOffset({x, 0.35f, z});
                parts.push_back(legShape);
            }
        table->setPos({-4.5f, 0.3f, 3.5f});
        this->add(table);
        m_world->addBody(table, Shape::compound(parts), {.mass = 15.0f, .friction = 0.7f});

        // Something on it
        addBox(glm::vec3(0.2f), {-4.3f, 1.4f, 3.5f}, m_red, 1.0f);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        lights->setUseShadows(true);
        lights->setAutoCalcBBox(false);
        lights->setSceneBoundingBox(glm::vec3(-12.0f, -1.0f, -12.0f), glm::vec3(12.0f, 8.0f, 12.0f));
        auto sun = lights->addDirectionalLight();
        sun->setAmbientColor(glm::vec3(0.3f));
        sun->setDiffuseColor(glm::vec3(1.0f));
        sun->setSpecularColor(glm::vec3(1.0f));
        sun->setDirection(glm::vec3(-0.4f, -1.0f, -0.5f));
        sun->setCastShadows(true);
        sun->setShadowMapSize(2048, 2048);
        sun->setShadowStrength(0.6f);
        lights->apply();

        m_wood = makeMaterial({0.7f, 0.5f, 0.3f, 1.0f});
        m_metal = makeMaterial({0.55f, 0.6f, 0.7f, 1.0f});
        m_stone = makeMaterial({0.6f, 0.6f, 0.55f, 1.0f});
        m_red = makeMaterial({0.85f, 0.3f, 0.2f, 1.0f});
        m_ball = makeMaterial({0.9f, 0.9f, 0.95f, 1.0f});

        m_world = PhysicsWorld::create();
        m_drag = DragController::create(m_world);
        m_world->setJointBreakCallback([this](const JointPtr &) { m_brokenWelds++; });

        auto ground = Box::create({12.0f, 0.1f, 12.0f});
        ground->setPos({0.0f, -0.1f, 0.0f});
        ground->setMaterial(makeMaterial({0.5f, 0.52f, 0.48f, 1.0f}));
        this->add(ground);
        if (auto body = m_world->addBody(ground, {.type = BodyType::Static, .friction = 0.8f}); !body)
            logError("Could not add the ground: " + body.error().message);

        buildBridge();
        buildChain();
        buildWindmill();
        buildPiston();
        buildTower();
        buildTable();

        m_debugOptions.colliders = false; // joints only, until asked for more
        m_debugOptions.contacts = false;

        cameraManipulator()->setCameraPosition(glm::vec3(1.0f, 7.5f, 16.0f));
        cameraManipulator()->setCameraTarget(glm::vec3(0.0f, 1.8f, 1.0f));
        cameraManipulator()->saveState();
        return 0;
    }

    void onUpdate() override
    {
        // The piston turns around at the ends of its stroke
        if (m_piston->translation() >= m_piston->upperLimit() - 0.02f && m_piston->motorSpeed() > 0.0f)
            m_piston->setMotor(-1.5f, 400.0f);
        else if (m_piston->translation() <= m_piston->lowerLimit() + 0.02f && m_piston->motorSpeed() < 0.0f)
            m_piston->setMotor(1.5f, 400.0f);

        m_world->step(frameTime());

        auto debug = renderContext()->debugDraw();
        if (m_debugView)
            drawDebug(*m_world, *debug, m_debugOptions);
        drawDebug(*m_drag, *debug);
    }

    void throwBall()
    {
        auto ball = Sphere::create(0.3);
        ball->setMaterial(m_ball);
        this->add(ball);
        auto body = m_world->addBody(ball, Shape::sphere(0.3f), {.mass = 6.0f, .bounciness = 0.2f});
        throwBody(body, cameraRay(mouseX(), mouseY()), 30.0f);
        m_thrown.push_back(body);
    }

    void resetScene()
    {
        m_drag->release();
        for (const auto &body : m_thrown)
        {
            scene()->remove(body->node());
            m_world->removeBody(body);
        }
        m_thrown.clear();
        m_world->reset();
        m_piston->setMotor(1.5f, 400.0f);
        m_brokenWelds = 0;
    }

    void onMouseButton(int button, int action, int mods) override
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse &&
                m_drag->grab(cameraRay(mouseX(), mouseY())))
                cameraManipulator()->setManipulationBlocked(true);
            else if (action == GLFW_RELEASE && m_drag->isDragging())
            {
                m_drag->release();
                cameraManipulator()->setManipulationBlocked(false);
            }
        }
        GLFWSceneWindow::onMouseButton(button, action, mods);
    }

    void onMousePosition(double x, double y) override
    {
        if (m_drag->isDragging())
            m_drag->drag(cameraRay(x, y));
        GLFWSceneWindow::onMousePosition(x, y);
    }

    void onDrawUi() override
    {
        ImGui::Begin("Joints");
        if (ImGui::Button(m_world->isPaused() ? "Run (P)" : "Pause (P)"))
            m_world->setPaused(!m_world->isPaused());
        ImGui::SameLine();
        if (ImGui::Button("Reset (R)"))
            resetScene();
        ImGui::SameLine();
        if (ImGui::Button("Throw a ball (Space)"))
            throwBall();
        ImGui::TextDisabled("Drag bodies with the left mouse button");

        ImGui::SeparatorText("Debug view");
        ImGui::Checkbox("Show (L)", &m_debugView);
        ImGui::Checkbox("Joints", &m_debugOptions.joints);
        ImGui::SameLine();
        ImGui::Checkbox("Colliders", &m_debugOptions.colliders);
        ImGui::SameLine();
        ImGui::Checkbox("Contacts", &m_debugOptions.contacts);

        ImGui::SeparatorText("Joints");
        ImGui::Text("%zu joints, %d broken", m_world->joints().size(), m_brokenWelds);
        ImGui::Text("Piston at %.2f", m_piston->translation());
        const PhysicsStats stats = m_world->stats();
        ImGui::Text("Bodies %d, awake %d, contacts %d", stats.bodies, stats.awakeBodies, stats.contacts);
        ImGui::End();
    }

    void onKey(int key, int scancode, int action, int mods) override
    {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_P)
                m_world->setPaused(!m_world->isPaused());
            else if (key == GLFW_KEY_R)
                resetScene();
            else if (key == GLFW_KEY_SPACE)
                throwBall();
            else if (key == GLFW_KEY_L)
                m_debugView = !m_debugView;
        }
        GLFWSceneWindow::onKey(key, scancode, action, mods);
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = JointsWindow::create(1280, 800, "Joints and compound bodies (ivfphysics)");
    app->addWindow(window);
    return app->loop();
}
