/**
 * @file rigid_body1.cpp
 * @brief ReactPhysics3D rigid body physics example
 * @author Jonas Lindemann
 * @example rigid_body1.cpp
 * @ingroup physics_examples
 *
 * This example demonstrates using ReactPhysics3D for rigid body physics simulation.
 * It creates a scene with falling boxes affected by gravity.
 */

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfui/scene_inspector.h>

#include <ivf/node_visitor.h>
#include <ivf/stock_shaders.h>

#include <reactphysics3d/reactphysics3d.h>

#include "physics_object.h"
#include "physics_box.h"

using namespace ivf;
using namespace ivfui;
using namespace std;

// Contact event listener to verify contact detection and resolution
class ContactListener : public rp3d::EventListener {
private:
    int m_contactCount = 0;
    int m_totalContactsThisFrame = 0;
    bool m_hasLoggedContact = false;



public:
    void onContact(const rp3d::CollisionCallback::CallbackData &callbackData) override
    {
        m_totalContactsThisFrame = 0;

        // Iterate through all contact pairs
        for (uint32_t p = 0; p < callbackData.getNbContactPairs(); p++)
        {
            rp3d::CollisionCallback::ContactPair contactPair = callbackData.getContactPair(p);

            // Iterate through all contact points
            for (uint32_t c = 0; c < contactPair.getNbContactPoints(); c++)
            {
                rp3d::CollisionCallback::ContactPoint contactPoint = contactPair.getContactPoint(c);
                m_totalContactsThisFrame++;

                // Log first contact for debugging (only once to avoid spam)
                if (!m_hasLoggedContact)
                {
                    rp3d::Vector3 normal = contactPoint.getWorldNormal();
                    float penetrationDepth = contactPoint.getPenetrationDepth();

                    std::cout << "\n=== FIRST CONTACT DETECTED ===" << std::endl;
                    std::cout << "  Contact Normal: (" << normal.x << ", " << normal.y << ", " << normal.z << ")"
                              << std::endl;
                    std::cout << "  Penetration Depth: " << penetrationDepth << std::endl;
                    std::cout << "================================\n" << std::endl;
                    m_hasLoggedContact = true;
                }
            }
        }

        m_contactCount = m_totalContactsThisFrame;
    }

    int getContactCount() const
    {
        return m_contactCount;
    }

    void reset()
    {
        m_contactCount = 0;
        m_totalContactsThisFrame = 0;
        m_hasLoggedContact = false;
    }
};

class ExampleWindow : public GLFWSceneWindow {
private:
    rp3d::PhysicsCommon m_physicsCommon;
    rp3d::PhysicsWorld *m_world;

    std::vector<PhysicsBoxPtr> m_physicsBoxes;
    std::vector<BoxPtr> m_visualBoxes;

    PhysicsBoxPtr m_ground;
    BoxPtr m_groundVisual;
    DirectionalLightPtr m_dirLight0;

    std::shared_ptr<ContactListener> m_contactListener;
    int m_frameCount = 0;

    bool m_paused = false;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

void setupPhysics()
    {
        std::cout << "\n=== PHYSICS SETUP ===" << std::endl;

        // Create a physics world with gravity
        rp3d::PhysicsWorld::WorldSettings settings;
        settings.defaultVelocitySolverNbIterations = 60;
        settings.defaultPositionSolverNbIterations = 60;
        settings.gravity = rp3d::Vector3(0, -9.81, 0);
        settings.isSleepingEnabled = true;

        m_world = m_physicsCommon.createPhysicsWorld(settings);

        // Create and register contact listener
        m_contactListener = std::make_shared<ContactListener>();
        m_world->setEventListener(m_contactListener.get());

        std::cout << "Physics World Configuration:" << std::endl;
        std::cout << "  Gravity: (0, -9.81, 0)" << std::endl;
        std::cout << "  Velocity Solver Iterations: " << m_world->getNbIterationsVelocitySolver() << std::endl;
        std::cout << "  Position Solver Iterations: " << m_world->getNbIterationsPositionSolver() << std::endl;
        std::cout << "  Sleeping Enabled: " << (m_world->isSleepingEnabled() ? "Yes" : "No") << std::endl;

        // Create ground plane (static box)
        // CORRECTED: Position ground so top surface is at Y=0
        std::cout << "\nCreating Ground:" << std::endl;
        m_ground = PhysicsBox::create(m_world, m_physicsCommon,
                                      glm::vec3(20.0f, 0.5f, 20.0f),     // Half-extents: thin ground plane
                                      glm::vec3(0.0f, -0.5f, 0.0f),      // Position: center at Y=-0.5, top at Y=0
                                      glm::quat(1.0f, 0.0f, 0.0f, 0.0f), // No rotation
                                      0.0f                               // Mass = 0 means static
        );
        m_ground->setRestitution(0.2f);
        m_ground->setFriction(0.5f);

        std::cout << "  Position: (0, -0.5, 0)" << std::endl;
        std::cout << "  Half-extents: (20, 0.5, 20) -> Full size: (40, 1, 40)" << std::endl;
        std::cout << "  Top surface at Y = 0.0" << std::endl;
        std::cout << "  Mass: " << m_ground->getMass() << " (STATIC)" << std::endl;
        std::cout << "  Restitution: " << m_ground->getRestitution() << std::endl;
        std::cout << "  Friction: " << m_ground->getFriction() << std::endl;

        std::cout << "\n=== PHYSICS SETUP COMPLETE ===" << std::endl;
    }


    void setupVisuals()
    {
        // Create materials
        auto boxMaterial = Material::create();
        boxMaterial->setDiffuseColor(glm::vec4(0.8f, 0.3f, 0.2f, 1.0f));

        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor(glm::vec4(0.3f, 0.6f, 0.3f, 1.0f));

        // Create visual ground (using Box instead of Cube for vec3 size support)
        m_groundVisual = Box::create(glm::vec3(20.0f, 0.01, 20.0f));
        m_groundVisual->setPos(glm::vec3(0.0f, 0.0f, 0.0f));   
        m_groundVisual->setMaterial(groundMaterial);
        this->add(m_groundVisual);

        // Create visual boxes for each physics box

        /*
        for (auto &physicsBox : m_physicsBoxes)
        {
            auto visualBox = Box::create(glm::vec3(1.0f, 1.0f, 1.0f)); // Full size (half-extents * 2)
            visualBox->setMaterial(boxMaterial);
            m_visualBoxes.push_back(visualBox);
            this->add(visualBox);
        }
        */
    }

    virtual int onSetup() override
    {
        auto lightManager = LightManager::instance();
        lightManager->clearLights();
        lightManager->setUseShadows(true);
        lightManager->setAutoCalcBBox(false);
        lightManager->setSceneBoundingBox(glm::vec3(-20.0, -20.0, -20.0), glm::vec3(20.0, 20.0, 20.0));

        m_dirLight0 = lightManager->addDirectionalLight();
        m_dirLight0->setAmbientColor(glm::vec3(0.3, 0.3, 0.3));
        m_dirLight0->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        m_dirLight0->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        m_dirLight0->setDirection(glm::vec3(-0.5, -1.0, -0.5));
        m_dirLight0->setEnabled(true);
        m_dirLight0->setCastShadows(true);
        m_dirLight0->setShadowMapSize(4096, 4096);
        m_dirLight0->setShadowStrength(0.3f); // Set shadow strength (0.0 to 1.0) for the directional light
        lightManager->apply();

        this->setupPhysics();

        //this->enableHeadlight();
        this->setAxisVisible(true);

        // Setup camera
        this->cameraManipulator()->setCameraPosition(glm::vec3(15.0f, 10.0f, 15.0f));
        this->cameraManipulator()->setFarZ(1000.0f);
        this->cameraManipulator()->saveState();

        // Set OpenGL state
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        this->setupVisuals();

        return 0;
    }

virtual void onUpdate() override
    {
        if (m_paused)
            return;

        // Use multiple smaller time steps for better accuracy
        const float fixedTimeStep = 1.0f / 120.0f; // 120 Hz instead of 60 Hz
        const int numSteps = 2;                    // Run 2 sub-steps per frame

        for (int i = 0; i < numSteps; ++i)
        {
            m_world->update(fixedTimeStep);
        }

        m_frameCount++;

        // Update visual positions from physics
        m_ground->updateFromPhysics();

        for (size_t i = 0; i < m_physicsBoxes.size(); i++)
        {
            m_physicsBoxes[i]->updateFromPhysics();

            glm::vec3 pos = m_physicsBoxes[i]->getPosition();
            glm::quat rot = m_physicsBoxes[i]->getRotation();
            glm::vec3 euler = glm::eulerAngles(rot);
            euler = glm::degrees(euler);

            m_visualBoxes[i]->setPos(pos);
            m_visualBoxes[i]->setEulerAngles(euler);
        }
    }

    virtual void onAddMenuItems(UiMenu *menu) override
    {
        auto resetItem = UiMenuItem::create("Reset Scene", "", [this]() { this->resetScene(); });
        menu->addItem(resetItem);
    }

    virtual void onDrawUi() override
    {
        if (ImGui::Button("Reset Scene"))
        {
            this->resetScene();
        }

        ImGui::Separator();
        ImGui::Text("Physics Diagnostics:");
        ImGui::Text("Frame: %d", m_frameCount);

        ImGui::Separator();
        ImGui::Text("Contact Information:");
        if (m_contactListener)
        {
            int contactCount = m_contactListener->getContactCount();
            ImGui::Text("Active Contacts: %d", contactCount);

            if (contactCount == 0)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WARNING: No contacts detected!");
                ImGui::TextWrapped("If boxes are falling through ground, collision detection is not working.");
            }
            else
            {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Contacts detected - solver is working");
            }
        }

        ImGui::Separator();
        ImGui::Text("Solver Configuration:");
        ImGui::Text("Velocity Iterations: %d", m_world->getNbIterationsVelocitySolver());
        ImGui::Text("Position Iterations: %d", m_world->getNbIterationsPositionSolver());

        ImGui::Separator();
        ImGui::Text("Box 0 State:");
        if (!m_physicsBoxes.empty())
        {
            auto &box = m_physicsBoxes[0];
            glm::vec3 pos = box->getPosition();
            glm::vec3 vel = box->getVelocity();

            ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
            ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);

            // Check if box has fallen through ground
            if (pos.y < -2.0f)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Box fell through ground!");
                ImGui::TextWrapped("Contact solver is NOT working properly.");
            }
        }
    }

    void resetScene()
    {
        std::cout << "\n=== SCENE RESET ===" << std::endl;
        m_contactListener->reset();
        m_frameCount = 0;

        // Reset box positions
        for (int i = 0; i < (int)m_physicsBoxes.size(); i++)
        {
            m_physicsBoxes[i]->setPosition(glm::vec3(-4.0f + i * 2.5f, 5.0f + i * 1.5f, 0.0f));
            m_physicsBoxes[i]->setVelocity(glm::vec3(0.0f));
            m_physicsBoxes[i]->setAngularVelocity(glm::vec3(0.0f));
        }

        std::cout << "Boxes reset. Waiting for first contact..." << std::endl;
    }

void addBox()
    {
        // Create a small random rotation for variety
        auto axis = glm::vec3(random(-1.0f, 1.0f), random(-1.0f, 1.0f), random(-1.0f, 1.0f));
        axis = glm::normalize(axis);
        auto quat = glm::angleAxis(random(0.0f, 0.5f), axis); // Small random rotation

        // Create physics box with explicit parameters
        auto box = PhysicsBox::create(m_world, m_physicsCommon,
                                      glm::vec3(1.0f, 1.0f, 1.0f), // Half-extents (creates 2x2x2 box)
                                      glm::vec3(random(-5.0f, 5.0f), 10.0f, random(-5.0f, 5.0f)), // Position
                                      quat,                                                       // Rotation
                                      1.0f                                                        // Mass (1 kg)
        );

        box->setRestitution(0.6f);
        box->setFriction(0.3f);

        m_physicsBoxes.push_back(box);

        // Create visual box
        auto boxMaterial = Material::create();
        boxMaterial->setDiffuseColor(glm::vec4(0.8f, 0.3f, 0.2f, 1.0f));

        auto visualBox = Box::create(glm::vec3(1.0f, 1.0f, 1.0f)); // Full size (2x half-extents)
        visualBox->setMaterial(boxMaterial);
        m_visualBoxes.push_back(visualBox);
        this->add(visualBox);
    }


    void onKey(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            this->resetScene();
        }

        if (key == GLFW_KEY_P && action == GLFW_PRESS)
        {
            m_paused = !m_paused;
            if (m_paused)
            {
                std::cout << "Physics simulation PAUSED." << std::endl;
            }
            else
            {
                std::cout << "Physics simulation RESUMED." << std::endl;
            }
        }

        if (key == GLFW_KEY_D && action == GLFW_PRESS)
        {
            this->addBox();
        }
        GLFWSceneWindow::onKey(key, scancode, action, mods);
    }

    void onExit()
    {
        this->close();
    }

    void onOpen()
    {}
};

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = ExampleWindow::create(1280, 800, "Rigid Body Physics - Contact Solver Test");

    app->addWindow(window);
    return app->loop();
}
