/**
 * @file rigid_body1.cpp
 * @brief Rigid body physics with ivfphysics: bodies on scene nodes.
 * @author Jonas Lindemann
 * @example rigid_body1.cpp
 * @ingroup physics_examples
 *
 * The minimal ivfphysics example:
 *  - a PhysicsWorld stepped with the window's frame time (fixed steps, interpolated nodes)
 *  - a static ground plane and dynamic boxes, spheres, capsules and capped cylinders whose
 *    shapes come from the nodes (Shape::fromNode(): a cylinder becomes a convex hull)
 *  - D drops another body, P pauses, S steps once, R resets to the start
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/logger.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <ivfphysics/ivfphysics.h>

#include <array>
#include <string>

using namespace ivf;
using namespace ivfui;
using namespace ivfphysics;

class RigidBodyWindow : public GLFWSceneWindow {
private:
    PhysicsWorldPtr m_world;
    std::array<MaterialPtr, 4> m_materials;
    int m_dropped{0};

public:
    RigidBodyWindow(int width, int height, const std::string &title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<RigidBodyWindow> create(int width, int height, const std::string &title)
    {
        return std::make_shared<RigidBodyWindow>(width, height, title);
    }

    /** A node of one of four kinds at @p pos, with a body whose shape comes from the node. */
    void addBody(int kind, const glm::vec3 &pos, const glm::vec3 &eulerAngles = glm::vec3(0.0f))
    {
        TransformNodePtr node;
        switch (kind % 4)
        {
        case 0:
            node = Box::create({0.4f, 0.4f, 0.4f}); // half extents
            break;
        case 1:
            node = Sphere::create(0.45);
            break;
        case 2:
            node = Capsule::create(0.3, 0.4);
            break;
        default:
            node = CappedCylinder::create(0.4, 0.8);
            break;
        }
        node->setPos(pos);
        node->setEulerAngles(eulerAngles);
        node->setMaterial(m_materials[kind % 4]);
        this->add(node);

        auto body = m_world->addBody(node, {.mass = 1.0f, .friction = 0.6f, .bounciness = 0.2f});
        if (!body)
            logError("Could not add a body: " + body.error().message);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        lights->setUseShadows(true);
        lights->setAutoCalcBBox(false);
        lights->setSceneBoundingBox(glm::vec3(-12.0f, -1.0f, -12.0f), glm::vec3(12.0f, 12.0f, 12.0f));

        auto sun = lights->addDirectionalLight();
        sun->setAmbientColor(glm::vec3(0.3f));
        sun->setDiffuseColor(glm::vec3(1.0f));
        sun->setSpecularColor(glm::vec3(1.0f));
        sun->setDirection(glm::vec3(-0.5f, -1.0f, -0.4f));
        sun->setCastShadows(true);
        sun->setShadowMapSize(2048, 2048);
        sun->setShadowStrength(0.6f);
        lights->apply();

        const std::array<glm::vec4, 4> colors{glm::vec4(0.85f, 0.35f, 0.2f, 1.0f), glm::vec4(0.25f, 0.55f, 0.85f, 1.0f),
                                              glm::vec4(0.95f, 0.75f, 0.2f, 1.0f), glm::vec4(0.4f, 0.75f, 0.35f, 1.0f)};
        for (std::size_t i = 0; i < colors.size(); ++i)
        {
            m_materials[i] = Material::create();
            m_materials[i]->setDiffuseColor(colors[i]);
            m_materials[i]->setSpecularColor(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
            m_materials[i]->setShininess(40.0f);
        }

        m_world = PhysicsWorld::create();

        // Ground: a thin box for the eye (half extents), a plane for the physics at its top, y = 0
        auto ground = Box::create({12.0f, 0.1f, 12.0f});
        ground->setPos({0.0f, -0.1f, 0.0f});
        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor(glm::vec4(0.55f, 0.55f, 0.5f, 1.0f));
        ground->setMaterial(groundMaterial);
        this->add(ground);
        m_world->addBody(ground, Shape::plane(), {.type = BodyType::Static, .friction = 0.8f});

        // A loose pile: four layers of bodies, slightly offset and tilted so they tumble
        for (int layer = 0; layer < 4; ++layer)
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                {
                    const float x = -2.4f + 1.6f * static_cast<float>(i) + 0.2f * static_cast<float>(layer % 2);
                    const float z = -2.4f + 1.6f * static_cast<float>(j) + 0.15f * static_cast<float>(layer);
                    const glm::vec3 tilt(15.0f * static_cast<float>(i - j), 20.0f * static_cast<float>(layer),
                                         10.0f * static_cast<float>(j));
                    addBody(i + j + layer, {x, 2.0f + 1.6f * static_cast<float>(layer), z}, tilt);
                }

        cameraManipulator()->setCameraPosition(glm::vec3(10.0f, 8.0f, 12.0f));
        cameraManipulator()->setCameraTarget(glm::vec3(0.0f, 1.0f, 0.0f));
        cameraManipulator()->saveState();
        return 0;
    }

    void onUpdate() override
    {
        m_world->step(frameTime());
    }

    void dropBody()
    {
        const float x = static_cast<float>(ivf::random(-2.0, 2.0));
        const float z = static_cast<float>(ivf::random(-2.0, 2.0));
        addBody(m_dropped++, {x, 9.0f, z}, {30.0f, 45.0f, 0.0f});
    }

    void onDrawUi() override
    {
        ImGui::Begin("Physics");
        if (ImGui::Button(m_world->isPaused() ? "Run (P)" : "Pause (P)"))
            m_world->setPaused(!m_world->isPaused());
        ImGui::SameLine();
        if (ImGui::Button("Step (S)"))
            m_world->stepOnce();
        ImGui::SameLine();
        if (ImGui::Button("Reset (R)"))
            m_world->reset();
        if (ImGui::Button("Drop a body (D)"))
            dropBody();

        float timeScale = static_cast<float>(m_world->timeScale());
        if (ImGui::SliderFloat("Time scale", &timeScale, 0.0f, 2.0f))
            m_world->setTimeScale(timeScale);
        glm::vec3 g = m_world->gravity();
        if (ImGui::SliderFloat("Gravity", &g.y, -20.0f, 0.0f))
            m_world->setGravity(g);

        ImGui::Text("Time %.2f s, %llu steps", m_world->time(), static_cast<unsigned long long>(m_world->stepCount()));
        ImGui::Text("Bodies %zu, awake %d", m_world->bodies().size(), m_world->awakeBodyCount());
        ImGui::End();
    }

    void onKey(int key, int scancode, int action, int mods) override
    {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_P)
                m_world->setPaused(!m_world->isPaused());
            else if (key == GLFW_KEY_S)
                m_world->stepOnce();
            else if (key == GLFW_KEY_R)
                m_world->reset();
            else if (key == GLFW_KEY_D)
                dropBody();
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

    auto window = RigidBodyWindow::create(1280, 800, "Rigid body physics (ivfphysics)");
    app->addWindow(window);
    return app->loop();
}
