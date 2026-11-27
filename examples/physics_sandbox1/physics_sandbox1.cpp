/**
 * @file physics_sandbox1.cpp
 * @brief An interactive physics sandbox: presets, throwing, dragging, colouring by speed.
 * @author Jonas Lindemann
 * @example physics_sandbox1.cpp
 * @ingroup physics_examples
 *
 * The complete ivfphysics application:
 *  - five presets (keys 1-5): a brick wall and a heavy ball, a domino run, a log tower and a
 *    wrecking ball, pendulums (a Newton's cradle on hinges, a chain on ball joints) and a
 *    ramp course of compound static bodies
 *  - drag bodies with the left mouse button (DragController); Ctrl + click or Space throws a ball
 *    through the mouse position (throwBody())
 *  - bodies coloured by their speed with an ivfvis colormap and colour bar; optional velocity
 *    arrows, contact points and colliders (drawDebug())
 *  - sliders for gravity, friction, bounciness, simulation speed and throw speed; P pauses,
 *    N steps, R rebuilds the preset, L toggles the debug view
 *  - PBR materials lit by a procedural sky (EnvironmentMap), shadows, ACES tone mapping
 * The simulation uses fixed steps and no randomness, so every run is the same.
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/logger.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <ivfphysics/ivfphysics.h>
#include <ivfvis/color_bar.h>
#include <ivfvis/colormap.h>

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;
using namespace ivfphysics;

namespace {

enum class Preset { BrickWall, Dominoes, Tower, Pendulums, Ramps };
constexpr std::array<const char *, 5> PresetNames{"Brick wall", "Domino run", "Tower and wrecking ball",
                                                  "Pendulums", "Ramp course"};

const glm::vec3 Up(0.0f, 1.0f, 0.0f);

} // namespace

class SandboxWindow : public GLFWSceneWindow {
private:
    /** A dynamic body with its own material, so it can be coloured by speed. */
    struct Item {
        RigidBodyPtr body;
        PBRMaterialPtr material;
        glm::vec4 color;
    };

    EnvironmentMapPtr m_environment;
    ivfvis::ColormapPtr m_colormap;
    ivfvis::ColorBarPtr m_colorBar;

    PhysicsWorldPtr m_world;
    DragControllerPtr m_drag;
    CompositeNodePtr m_content;
    std::vector<Item> m_items;
    std::vector<JointPtr> m_ropes; ///< Joints drawn as strings.
    std::vector<RigidBodyPtr> m_thrown;
    Preset m_preset{Preset::BrickWall};

    // Settings (kept when switching presets)
    float m_gravity{9.81f};
    float m_friction{0.5f};
    float m_bounciness{0.2f};
    float m_timeScale{1.0f};
    float m_throwSpeed{20.0f};
    float m_maxSpeed{8.0f};
    bool m_colorBySpeed{true};
    bool m_debugView{false};
    DebugViewOptions m_debugOptions;

public:
    SandboxWindow(int width, int height, const std::string &title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<SandboxWindow> create(int width, int height, const std::string &title)
    {
        return std::make_shared<SandboxWindow>(width, height, title);
    }

    // --- Building blocks ----------------------------------------------------------------------

    PBRMaterialPtr makeMaterial(const glm::vec4 &color, float roughness = 0.5f, float metallic = 0.0f)
    {
        auto material = PBRMaterial::create();
        material->setAlbedo(color);
        material->setRoughness(roughness);
        material->setMetallic(metallic);
        material->setEnvironment(m_environment);
        material->setToneMapping(false); // linear HDR: the window tone maps
        return material;
    }

    /** Add @p node to the scene with a dynamic body. */
    RigidBodyPtr addDynamic(const TransformNodePtr &node, const ShapePtr &shape, const glm::vec4 &color, float mass,
                            float roughness = 0.5f, float metallic = 0.0f)
    {
        auto material = makeMaterial(color, roughness, metallic);
        node->setMaterial(material);
        m_content->add(node);
        auto body = m_world->addBody(node, shape, {.mass = mass, .friction = m_friction, .bounciness = m_bounciness});
        m_items.push_back({body, material, color});
        return body;
    }

    /** Add @p node to the scene with a static body. */
    RigidBodyPtr addStatic(const TransformNodePtr &node, const ShapePtr &shape, const glm::vec4 &color,
                           float roughness = 0.8f, float friction = 0.7f)
    {
        if (!std::dynamic_pointer_cast<CompositeNode>(node))
            node->setMaterial(makeMaterial(color, roughness));
        m_content->add(node);
        return m_world->addBody(node, shape, {.type = BodyType::Static, .friction = friction});
    }

    RigidBodyPtr addBox(const glm::vec3 &half, const glm::vec3 &pos, const glm::vec4 &color, float mass,
                        const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
    {
        auto node = Box::create(half);
        node->setPos(pos);
        node->setRotation(rotation);
        return addDynamic(node, Shape::box(half), color, mass);
    }

    RigidBodyPtr addBall(float radius, const glm::vec3 &pos, const glm::vec4 &color, float mass, float metallic = 0.0f)
    {
        auto node = Sphere::create(radius, 32, 16);
        node->setPos(pos);
        return addDynamic(node, Shape::sphere(radius), color, mass, metallic > 0.0f ? 0.25f : 0.5f, metallic);
    }

    // --- Presets ------------------------------------------------------------------------------

    void buildBrickWall()
    {
        constexpr int rows = 9;
        constexpr int columns = 8;
        const glm::vec3 half(0.299f, 0.149f, 0.2f);
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < columns; ++c)
            {
                const float x = 0.6f * (static_cast<float>(c) - 0.5f * (columns - 1)) + (r % 2 ? 0.15f : -0.15f);
                const float shade = 0.85f + 0.1f * static_cast<float>((r + c) % 3) / 2.0f;
                addBox(half, {x, 0.15f + 0.3f * static_cast<float>(r), 0.0f},
                       glm::vec4(0.75f * shade, 0.32f * shade, 0.22f * shade, 1.0f), 1.5f);
            }
        auto ball = addBall(0.55f, {0.3f, 1.1f, 8.0f}, {0.8f, 0.8f, 0.85f, 1.0f}, 40.0f, 1.0f);
        ball->setLinearVelocity({0.0f, 0.0f, -14.0f});
        setCamera({7.0f, 4.5f, 10.0f}, {0.0f, 1.3f, 0.0f});
    }

    void buildDominoes()
    {
        // Along an S-curve, about 0.45 apart
        const glm::vec3 half(0.25f, 0.5f, 0.06f);
        auto curve = [](float t) { return glm::vec3(-7.0f + 14.0f * t, 0.0f, 2.5f * std::sin(glm::two_pi<float>() * t)); };
        float t = 0.0f;
        glm::vec3 previous = curve(0.0f);
        bool first = true;
        int count = 0;
        while (t < 1.0f)
        {
            // Step along the curve until the next domino position
            glm::vec3 p = previous;
            float travelled = 0.0f;
            while (!first && travelled < 0.45f && t < 1.0f)
            {
                t += 0.0005f;
                const glm::vec3 q = curve(t);
                travelled += glm::length(q - p);
                p = q;
            }
            if (t >= 1.0f)
                break;
            const glm::vec3 d = glm::normalize(curve(std::min(t + 0.001f, 1.0f)) - curve(t));
            const glm::quat facing = glm::angleAxis(std::atan2(d.x, d.z), Up); // thin side along the curve
            const float hue = static_cast<float>(count % 6) / 6.0f;
            const glm::vec4 color(0.35f + 0.5f * hue, 0.45f, 0.85f - 0.5f * hue, 1.0f);
            auto domino = addBox(half, {p.x, half.y, p.z}, color, 0.6f, facing);
            if (first)
                domino->setAngularVelocity(2.5f * glm::cross(Up, d)); // tip the first one forward
            first = false;
            previous = p;
            ++count;
        }
        setCamera({0.0f, 10.0f, 13.0f}, {0.0f, 0.0f, 0.5f});
    }

    void buildTower()
    {
        // Logs, three per layer, crossing layer by layer
        constexpr int layers = 14;
        for (int layer = 0; layer < layers; ++layer)
            for (int i = -1; i <= 1; ++i)
            {
                const float y = 0.15f + 0.3f * static_cast<float>(layer);
                const bool alongX = layer % 2 == 0;
                const glm::vec3 half = alongX ? glm::vec3(0.75f, 0.149f, 0.245f) : glm::vec3(0.245f, 0.149f, 0.75f);
                const glm::vec3 pos = alongX ? glm::vec3(0.0f, y, 0.5f * static_cast<float>(i))
                                             : glm::vec3(0.5f * static_cast<float>(i), y, 0.0f);
                const float shade = 0.8f + 0.1f * static_cast<float>((layer + i + 1) % 3);
                addBox(half, pos, glm::vec4(0.72f * shade, 0.52f * shade, 0.3f * shade, 1.0f), 2.0f);
            }

        // A wrecking ball on a ball and socket joint, raised to the side
        const glm::vec3 pivot(0.0f, 9.0f, 0.0f);
        constexpr float length = 6.6f;
        const float angle = glm::radians(75.0f);
        auto ball = addBall(0.6f, pivot + length * glm::vec3(-std::sin(angle), -std::cos(angle), 0.0f),
                            {0.2f, 0.2f, 0.22f, 1.0f}, 80.0f, 1.0f);
        m_ropes.push_back(m_world->addBallSocketJoint(ball, nullptr, pivot));
        setCamera({8.0f, 6.0f, 11.0f}, {-1.0f, 2.5f, 0.0f});
    }

    void buildPendulums()
    {
        // Newton's cradle: balls on hinges, the first one raised. Small gaps between the balls: the
        // collisions then happen one after another, which is what passes the impulse along the row
        // (touching balls are solved together and the first one bounces back)
        const glm::vec4 frameColor(0.3f, 0.3f, 0.33f, 1.0f);
        constexpr float length = 2.5f;
        constexpr float radius = 0.3f;
        constexpr float top = 4.0f;
        // The frame is wide enough for the outer balls to swing out
        auto beam = Box::create({3.8f, 0.06f, 0.06f});
        beam->setPos({0.0f, top, 0.0f});
        addStatic(beam, Shape::box({3.8f, 0.06f, 0.06f}), frameColor, 0.4f);
        for (const float x : {-3.8f, 3.8f})
        {
            auto post = Box::create({0.06f, 0.5f * top, 0.06f});
            post->setPos({x, 0.5f * top, 0.0f});
            addStatic(post, Shape::box({0.06f, 0.5f * top, 0.06f}), frameColor, 0.4f);
        }
        for (int i = 0; i < 5; ++i)
        {
            const float x = (2.0f * radius + 0.01f) * (static_cast<float>(i) - 2.0f);
            const glm::vec3 pivot(x, top, 0.0f);
            const float angle = i == 0 ? glm::radians(50.0f) : 0.0f;
            auto ball = addBall(radius, pivot + length * glm::vec3(-std::sin(angle), -std::cos(angle), 0.0f),
                                {0.85f, 0.85f, 0.9f, 1.0f}, 1.0f, 1.0f);
            ball->setFriction(0.0f);
            ball->setBounciness(1.0f);
            ball->setDamping(0.0f, 0.0f);
            ball->setCanSleep(false);
            m_ropes.push_back(m_world->addHingeJoint(ball, nullptr, pivot, {0.0f, 0.0f, 1.0f}));
        }

        // A chain of ten links on ball joints, released horizontally
        const glm::vec3 hook(9.0f, 5.0f, -2.0f);
        RigidBodyPtr previous;
        for (int i = 1; i <= 10; ++i)
        {
            const glm::vec3 pos = hook + glm::vec3(-0.45f * static_cast<float>(i), 0.0f, 0.0f);
            auto link = addBall(0.17f, pos, {0.9f, 0.6f, 0.2f, 1.0f}, 0.5f, 1.0f);
            link->setCanSleep(false);
            const glm::vec3 anchor = pos + glm::vec3(0.45f, 0.0f, 0.0f);
            m_ropes.push_back(previous ? m_world->addBallSocketJoint(previous, link, anchor)
                                       : m_world->addBallSocketJoint(link, nullptr, anchor));
            previous = link;
        }
        setCamera({2.5f, 5.0f, 15.0f}, {2.5f, 2.5f, 0.0f});
    }

    /** A static ramp: one body whose compound shape is the floor, two rails and a back wall. */
    void addRamp(const glm::vec3 &center, float slope)
    {
        const glm::vec4 color(0.7f, 0.55f, 0.36f, 1.0f);
        struct Part {
            glm::vec3 half;
            glm::vec3 offset;
        };
        const float backX = slope > 0.0f ? -3.6f : 3.6f; // the high end
        const std::array<Part, 4> parts{Part{{3.6f, 0.08f, 0.9f}, {0.0f, 0.0f, 0.0f}},
                                        Part{{3.6f, 0.25f, 0.05f}, {0.0f, 0.25f, 0.95f}},
                                        Part{{3.6f, 0.25f, 0.05f}, {0.0f, 0.25f, -0.95f}},
                                        Part{{0.05f, 0.9f, 0.9f}, {backX, 0.9f, 0.0f}}};
        auto node = CompositeNode::create();
        auto material = makeMaterial(color, 0.6f);
        std::vector<ShapePtr> shapes;
        for (const auto &part : parts)
        {
            auto box = Box::create(part.half);
            box->setPos(part.offset);
            box->setMaterial(material);
            node->add(box);
            auto shape = Shape::box(part.half);
            shape->setOffset(part.offset);
            shapes.push_back(shape);
        }
        node->setPos(center);
        node->setRotation(glm::angleAxis(-slope, glm::vec3(0.0f, 0.0f, 1.0f)));
        addStatic(node, Shape::compound(shapes), color, 0.8f, 0.3f);
    }

    void buildRamps()
    {
        // Zig-zag: every second ramp is shifted so its high end (with the back wall) reaches past
        // the low end of the ramp above and catches what flies off it: its back wall reaches above that end.
        // 3.5 apart, so the bodies roll back under the end of the ramp above with room to spare
        const float slope = glm::radians(18.0f);
        for (int i = 0; i < 3; ++i)
            addRamp({i % 2 == 0 ? 0.0f : 1.0f, 11.0f - 3.5f * static_cast<float>(i), 0.0f}, i % 2 == 0 ? slope : -slope);

        // A bin where the bodies fly off the last ramp
        const glm::vec4 binColor(0.4f, 0.42f, 0.45f, 1.0f);
        const glm::vec3 binCenter(6.8f, 0.0f, 0.0f);
        const std::array<std::pair<glm::vec3, glm::vec3>, 4> walls{
            std::pair{glm::vec3(0.05f, 0.3f, 1.8f), glm::vec3(-1.8f, 0.3f, 0.0f)},
            std::pair{glm::vec3(0.05f, 0.3f, 1.8f), glm::vec3(1.8f, 0.3f, 0.0f)},
            std::pair{glm::vec3(1.8f, 0.3f, 0.05f), glm::vec3(0.0f, 0.3f, 1.8f)},
            std::pair{glm::vec3(1.8f, 0.3f, 0.05f), glm::vec3(0.0f, 0.3f, -1.8f)}};
        for (const auto &[half, offset] : walls)
        {
            auto wall = Box::create(half);
            wall->setPos(binCenter + offset);
            addStatic(wall, Shape::box(half), binColor, 0.5f);
        }

        // Balls, capsules and cylinders in two lanes down the first ramp, lying across it so they roll
        const std::array colors{glm::vec4(0.9f, 0.3f, 0.25f, 1.0f), glm::vec4(0.95f, 0.75f, 0.2f, 1.0f),
                                glm::vec4(0.3f, 0.7f, 0.4f, 1.0f)};
        const glm::quat across = glm::angleAxis(glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
        for (int k = 0; k < 10; ++k)
        {
            const float x = -3.0f + 0.5f * static_cast<float>(k / 2);
            const glm::vec3 pos(x, 11.0f - 0.325f * x + 0.45f, k % 2 == 0 ? -0.45f : 0.45f);
            const glm::vec4 color = colors[static_cast<std::size_t>(k % 3)];
            TransformNodePtr node;
            if (k % 3 == 0)
                node = Sphere::create(0.22, 32, 16);
            else if (k % 3 == 1)
                node = Capsule::create(0.15, 0.15); // radius, half length of the middle
            else
                node = CappedCylinder::create(0.2, 0.3);
            node->setPos(pos);
            if (k % 3 != 0)
                node->setRotation(across);
            if (auto shape = Shape::fromNode(node)) // the cylinder becomes a convex hull
                addDynamic(node, *shape, color, 1.0f);
        }
        setCamera({7.0f, 13.0f, 21.0f}, {1.5f, 5.0f, 0.0f});
    }

    // --- World ----------------------------------------------------------------------------------

    void setCamera(const glm::vec3 &position, const glm::vec3 &target)
    {
        cameraManipulator()->setCameraPosition(position);
        cameraManipulator()->setCameraTarget(target);
        cameraManipulator()->saveState();
    }

    /** A new world with the preset's bodies (also how R resets). */
    void loadPreset(Preset preset)
    {
        m_preset = preset;
        if (m_drag)
            m_drag->release();
        cameraManipulator()->setManipulationBlocked(false);
        if (m_content)
            scene()->remove(m_content);
        m_content = CompositeNode::create();
        this->add(m_content);
        m_items.clear();
        m_ropes.clear();
        m_thrown.clear();

        m_world = PhysicsWorld::create({.gravity = {0.0f, -m_gravity, 0.0f}});
        m_world->setTimeScale(m_timeScale);
        m_drag = DragController::create(m_world);

        auto ground = Box::create({15.0f, 0.1f, 15.0f});
        ground->setPos({0.0f, -0.1f, 0.0f});
        addStatic(ground, Shape::box({15.0f, 0.1f, 15.0f}), {0.55f, 0.56f, 0.52f, 1.0f}, 0.9f);

        switch (preset)
        {
        case Preset::BrickWall:
            buildBrickWall();
            break;
        case Preset::Dominoes:
            buildDominoes();
            break;
        case Preset::Tower:
            buildTower();
            break;
        case Preset::Pendulums:
            buildPendulums();
            break;
        case Preset::Ramps:
            buildRamps();
            break;
        }
    }

    void throwBall()
    {
        auto body = addBall(0.3f, glm::vec3(0.0f), {0.85f, 0.85f, 0.9f, 1.0f}, 6.0f, 1.0f);
        throwBody(body, cameraRay(mouseX(), mouseY()), m_throwSpeed);
        m_thrown.push_back(body);
        if (m_thrown.size() > 40) // keep the scene light: the oldest ball goes
            removeItem(m_thrown.front());
    }

    void removeItem(const RigidBodyPtr &body)
    {
        m_content->remove(body->node());
        m_world->removeBody(body);
        m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [&](const Item &i) { return i.body == body; }),
                      m_items.end());
        m_thrown.erase(std::remove(m_thrown.begin(), m_thrown.end(), body), m_thrown.end());
    }

    void applyMaterialSettings()
    {
        for (auto &item : m_items)
        {
            item.body->setFriction(m_friction);
            item.body->setBounciness(m_bounciness);
            item.body->wakeUp();
        }
    }

    // --- Window -------------------------------------------------------------------------------

    int onSetup() override
    {
        SkySettings sky;
        m_environment = EnvironmentMap::create();
        if (!m_environment->loadSky(sky))
            return -1;
        add(Skybox::create(m_environment->cubemap()));

        auto lights = renderContext()->lights();
        lights->clearLights();
        lights->setUseShadows(true);
        lights->setAutoCalcBBox(false);
        lights->setSceneBoundingBox(glm::vec3(-15.0f, -1.0f, -15.0f), glm::vec3(15.0f, 12.0f, 15.0f));
        auto sun = lights->addDirectionalLight();
        sun->setAmbientColor(glm::vec3(0.0f));
        sun->setDiffuseColor(sky.sunColor * 2.5f);
        sun->setSpecularColor(sky.sunColor);
        sun->setDirection(-glm::normalize(sky.sunDirection));
        sun->setCastShadows(true);
        sun->setShadowMapSize(2048, 2048);
        sun->setShadowStrength(0.7f);
        lights->apply();

        setToneMapping(ToneMapping::Aces);
        setGamma(2.2f);

        // Slate blue at rest (not black: bodies at rest stay readable), red at speed
        m_colormap = ivfvis::Colormap::create("speed", {{0.0f, {0.42f, 0.5f, 0.68f, 1.0f}},
                                                        {0.35f, {0.3f, 0.75f, 0.45f, 1.0f}},
                                                        {0.65f, {0.98f, 0.8f, 0.2f, 1.0f}},
                                                        {1.0f, {0.9f, 0.18f, 0.12f, 1.0f}}});
        m_colormap->setRange(0.0f, m_maxSpeed);
        m_colorBar = ivfvis::ColorBar::create(m_colormap);
        m_colorBar->setTitle("Speed [m/s]");
        add(m_colorBar);

        m_debugOptions.colliders = false;
        m_debugOptions.contacts = true;
        m_debugOptions.velocities = true;

        // IVF2_SANDBOX_PRESET=1..5 starts with another preset (in the browser: ?IVF2_SANDBOX_PRESET=3)
        Preset start = Preset::BrickWall;
        if (const char *value = std::getenv("IVF2_SANDBOX_PRESET"))
            if (const int n = std::atoi(value); n >= 1 && n <= static_cast<int>(PresetNames.size()))
                start = static_cast<Preset>(n - 1);
        loadPreset(start);
        return 0;
    }

    void onUpdate() override
    {
        m_world->step(frameTime());

        // Bodies that fell off the ground are removed
        std::vector<RigidBodyPtr> fallen;
        for (const auto &item : m_items)
            if (item.body->position().y < -20.0f)
                fallen.push_back(item.body);
        for (const auto &body : fallen)
            removeItem(body);

        // Colour by speed (or the body's own colour)
        m_colorBar->setVisible(m_colorBySpeed);
        for (auto &item : m_items)
            item.material->setAlbedo(m_colorBySpeed ? m_colormap->color(glm::length(item.body->linearVelocity()))
                                                    : item.color);

        auto debug = renderContext()->debugDraw();
        for (const auto &rope : m_ropes)
        {
            if (!rope->isValid() || rope->isBroken())
                continue;
            debug->line(rope->anchor(), rope->body1()->position(), {0.15f, 0.15f, 0.15f, 1.0f});
            if (auto other = rope->body2())
                debug->line(rope->anchor(), other->position(), {0.15f, 0.15f, 0.15f, 1.0f});
        }
        if (m_debugView)
            drawDebug(*m_world, *debug, m_debugOptions);
        drawDebug(*m_drag, *debug);
    }

    void onMouseButton(int button, int action, int mods) override
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && !ImGui::GetIO().WantCaptureMouse)
        {
            if (action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL))
                throwBall();
            else if (action == GLFW_PRESS && m_drag->grab(cameraRay(mouseX(), mouseY())))
                cameraManipulator()->setManipulationBlocked(true); // the mouse drags the body
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && m_drag->isDragging())
        {
            m_drag->release();
            cameraManipulator()->setManipulationBlocked(false);
        }
        GLFWSceneWindow::onMouseButton(button, action, mods);
    }

    void onMousePosition(double x, double y) override
    {
        if (m_drag->isDragging())
            m_drag->drag(cameraRay(x, y));
        GLFWSceneWindow::onMousePosition(x, y);
    }

    void onScroll(double xoffset, double yoffset) override
    {
        // While dragging, the wheel moves the body nearer or farther
        if (m_drag->isDragging())
        {
            m_drag->setDistance(m_drag->distance() * (1.0f + 0.1f * static_cast<float>(yoffset)));
            return;
        }
        GLFWSceneWindow::onScroll(xoffset, yoffset);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({10.0f, 10.0f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({330.0f, 0.0f}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Physics sandbox");

        int preset = static_cast<int>(m_preset);
        if (ImGui::Combo("Preset", &preset, PresetNames.data(), static_cast<int>(PresetNames.size())))
            loadPreset(static_cast<Preset>(preset));
        if (ImGui::Button(m_world->isPaused() ? "Run (P)" : "Pause (P)"))
            m_world->setPaused(!m_world->isPaused());
        ImGui::SameLine();
        if (ImGui::Button("Step (N)"))
            m_world->stepOnce();
        ImGui::SameLine();
        if (ImGui::Button("Reset (R)"))
            loadPreset(m_preset);
        ImGui::SameLine();
        if (ImGui::Button("Throw (Space)"))
            throwBall();
        ImGui::TextDisabled("Drag: left mouse (wheel: nearer/farther)\nThrow: Ctrl + click or Space");

        ImGui::SeparatorText("World");
        if (ImGui::SliderFloat("Gravity", &m_gravity, 0.0f, 20.0f, "%.2f m/s2"))
            m_world->setGravity({0.0f, -m_gravity, 0.0f});
        if (ImGui::SliderFloat("Friction", &m_friction, 0.0f, 1.5f))
            applyMaterialSettings();
        if (ImGui::SliderFloat("Bounciness", &m_bounciness, 0.0f, 1.0f))
            applyMaterialSettings();
        if (ImGui::SliderFloat("Speed", &m_timeScale, 0.05f, 2.0f, "%.2fx"))
            m_world->setTimeScale(m_timeScale);
        ImGui::SliderFloat("Throw speed", &m_throwSpeed, 5.0f, 50.0f, "%.0f m/s");

        ImGui::SeparatorText("Display");
        ImGui::Checkbox("Colour by speed", &m_colorBySpeed);
        if (m_colorBySpeed && ImGui::SliderFloat("Top speed", &m_maxSpeed, 1.0f, 30.0f, "%.0f m/s"))
            m_colormap->setRange(0.0f, m_maxSpeed);
        ImGui::Checkbox("Debug view (L)", &m_debugView);
        if (m_debugView)
        {
            ImGui::Checkbox("Velocities", &m_debugOptions.velocities);
            ImGui::SameLine();
            ImGui::Checkbox("Contacts", &m_debugOptions.contacts);
            ImGui::SameLine();
            ImGui::Checkbox("Colliders", &m_debugOptions.colliders);
        }

        const PhysicsStats stats = m_world->stats();
        ImGui::SeparatorText("Statistics");
        ImGui::Text("Time %.2f s", m_world->time());
        ImGui::Text("Bodies %d (%d awake), joints %zu", stats.dynamicBodies, stats.awakeBodies, m_world->joints().size());
        ImGui::Text("Contacts %d (%d points)", stats.contacts, stats.contactPoints);
        ImGui::Text("Physics %.2f ms per frame", stats.averageUpdateTimeMs);
        ImGui::End();
    }

    void onKey(int key, int scancode, int action, int mods) override
    {
        if (action == GLFW_PRESS)
        {
            if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5)
                loadPreset(static_cast<Preset>(key - GLFW_KEY_1));
            else if (key == GLFW_KEY_P)
                m_world->setPaused(!m_world->isPaused());
            else if (key == GLFW_KEY_N)
                m_world->stepOnce();
            else if (key == GLFW_KEY_R)
                loadPreset(m_preset);
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

    auto window = SandboxWindow::create(1280, 800, "Physics sandbox (ivfphysics)");
    app->addWindow(window);
    return app->loop();
}
