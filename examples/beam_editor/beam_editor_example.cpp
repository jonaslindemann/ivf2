/**
 * @file beam_editor_example.cpp
 * @brief 3D Beam Editor Example using Enhanced Scene Window
 * @author Jonas Lindemann
 * @example beam_editor_example.cpp
 * @ingroup placement_examples
 *
 * This example demonstrates a 3D beam application editor using the enhanced scene window
 * with 3D placement and manipulation capabilities. Users can:
 * - Place nodes in 3D space using mouse clicks
 * - Switch between XZ, XY, and YZ placement planes
 * - Move existing nodes by selecting and dragging them
 * - Create beam elements between nodes
 * - Toggle various visual aids like cursor and placement grid
 */

#include <iostream>
#include <memory>
#include <vector>
#include <map>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/material.h>
#include <ivf/cylinder.h>
#include <ivf/sphere.h>

#include <ivfui/ui.h>
#include <ivfui/enhanced_scene_window.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

/**
 * @class BeamNode
 * @brief Represents a structural node in the beam system
 */
class BeamNode {
public:
    int id;
    glm::vec3 position;
    SpherePtr visual;

    BeamNode(int nodeId, const glm::vec3 &pos) : id(nodeId), position(pos)
    {
        visual = Sphere::create();
        visual->setRadius(0.05f);
        visual->setPos(pos);
        visual->refresh();

        // Create node material
        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green
        material->setAmbientColor(glm::vec4(0.1f, 0.4f, 0.1f, 1.0f));
        visual->setMaterial(material);
    }
};

/**
 * @class BeamElement
 * @brief Represents a beam element connecting two nodes
 */
class BeamElement {
public:
    int id;
    int nodeA, nodeB;
    CylinderPtr visual;

    BeamElement(int elemId, int nodeIdA, int nodeIdB) : id(elemId), nodeA(nodeIdA), nodeB(nodeIdB)
    {
        visual = Cylinder::create();
        visual->setRadius(0.05f);

        // Create beam material
        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(0.8f, 0.2f, 0.2f, 1.0f)); // Red
        material->setAmbientColor(glm::vec4(0.4f, 0.1f, 0.1f, 1.0f));
        visual->setMaterial(material);
    }

    void updateGeometry(const glm::vec3 &posA, const glm::vec3 &posB)
    {
        glm::vec3 center = (posA + posB) * 0.5f;
        glm::vec3 direction = posB - posA;
        float length = glm::length(direction);

        visual->setPos(center);
        visual->setHeight(length);

        if (length > 1e-6f)
        {
            direction = glm::normalize(direction);

            // Calculate rotation to align cylinder with direction
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(up, direction));
            glm::vec3 realUp = glm::cross(direction, right);

            // Create rotation matrix
            glm::mat3 rotMat;
            rotMat[0] = right;
            rotMat[1] = direction; // Cylinder's default direction is Y
            rotMat[2] = realUp;

            // Convert to quaternion (simplified approach)
            glm::vec3 euler = glm::eulerAngles(glm::quat_cast(rotMat));
            visual->setEulerAngles(euler);
        }
    }
};

/**
 * @class BeamEditorWindow
 * @brief Main window for the 3D beam editor application
 */
class BeamEditorWindow : public EnhancedSceneWindow {
private:
    std::map<int, std::unique_ptr<BeamNode>> m_nodes;
    std::map<int, std::unique_ptr<BeamElement>> m_beams;
    int m_nextNodeId;
    int m_nextBeamId;

    // Selection state for beam creation
    int m_selectedNodeForBeam;
    bool m_creatingBeam;

    // Materials
    MaterialPtr m_selectedNodeMaterial;
    MaterialPtr m_normalNodeMaterial;

public:
    BeamEditorWindow(int width, int height, std::string title)
        : EnhancedSceneWindow(width, height, title), m_nextNodeId(1), m_nextBeamId(1), m_selectedNodeForBeam(-1),
          m_creatingBeam(false)
    {}

    static std::shared_ptr<BeamEditorWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<BeamEditorWindow>(width, height, title);
    }

    int onSetup() override
    {
        // Call base setup first
        /*
        int result = EnhancedSceneWindow::onSetup();
        if (result != 0)
        {
            return result;
        }
        */

        // Create materials
        m_normalNodeMaterial = Material::create();
        m_normalNodeMaterial->setDiffuseColor(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));
        m_normalNodeMaterial->setAmbientColor(glm::vec4(0.1f, 0.4f, 0.1f, 1.0f));

        m_selectedNodeMaterial = Material::create();
        m_selectedNodeMaterial->setDiffuseColor(glm::vec4(1.0f, 1.0f, 0.2f, 1.0f)); // Yellow
        m_selectedNodeMaterial->setAmbientColor(glm::vec4(0.5f, 0.5f, 0.1f, 1.0f));

        // Set up custom node creation callback
        this->setNodeCreationCallback([this](float x, float y, float z) -> ivf::NodePtr {
            cout << "Creating node at (" << x << ", " << y << ", " << z
                 << ") - ImGui capture: " << (this->placementMixin()->isImGuiCapturingMouse() ? "true" : "false")
                 << endl;
            return this->createBeamNode(glm::vec3(x, y, z));
        });

        // Set up node selection callback
        this->setNodeSelectionCallback([this](ivf::NodePtr node) { this->selectNodeForOperation(node); });

        // Set up node moved callback
        this->setNodeMovedCallback([this](ivf::NodePtr node, float x, float y, float z) {
            this->updateNodePosition(node, glm::vec3(x, y, z));
        });

        // Enable placement by default
        this->enablePlacement(true);

        // Configure placement settings
        this->setGridSnapping(true, 0.5f);
        this->setCursorVisible(true);

        cout << "=== 3D Beam Editor Controls ===" << endl;
        cout << "CTRL+P: Toggle placement mode" << endl;
        cout << "Left Click: Create node (CREATE mode) or Select node (MOVE mode)" << endl;
        cout << "  Note: Node creation is disabled when mouse is over UI elements" << endl;
        cout << "B: Toggle beam creation mode" << endl;
        cout << "SHIFT+X/Y/Z: Switch placement planes (grid follows automatically)" << endl;
        cout << "TAB: Cycle interaction modes (Auto/Camera/Placement)" << endl;
        cout << "ALT+1/2/3: Quick switch to Auto/Camera/Placement modes" << endl;
        cout << "L: Toggle plane constraint (default: ON)" << endl;
        cout << "SHIFT (while constrained): Lock in-plane position, move perpendicular" << endl;
        cout << "G: Toggle placement grid" << endl;
        cout << "C: Toggle cursor visibility" << endl;
        cout << "CTRL+G: Toggle grid snapping" << endl;
        cout << "M: Switch between CREATE/MOVE modes" << endl;
        cout << "DELETE: Delete selected elements" << endl;
        cout << "===============================" << endl;
        cout << "Interaction Modes:" << endl;
        cout << "  Auto: Context-sensitive (default)" << endl;
        cout << "  Camera: Mouse controls camera only" << endl;
        cout << "  Placement: Mouse controls node placement only" << endl;
        cout << "===============================" << endl;
        cout << "Plane Constraint Workflow (Default: ON):" << endl;
        cout << "  1. Move cursor freely in current plane (e.g., XZ)" << endl;
        cout << "  2. Hold SHIFT to lock in-plane position (X,Z locked)" << endl;
        cout << "  3. Move mouse up/down to adjust perpendicular axis (Y)" << endl;
        cout << "  4. Release SHIFT to continue in-plane movement at new level" << endl;
        cout << "  RED cursor: Constrained to plane" << endl;
        cout << "  ORANGE cursor: SHIFT held - perpendicular movement only" << endl;
        cout << "  YELLOW cursor: Plane constraint OFF - free 3D movement" << endl;
        cout << "===============================" << endl;

        return 0;
    }

    void onKey(int key, int scancode, int action, int mods) override
    {
        if (action == GLFW_PRESS)
        {
            // Toggle beam creation mode
            if (key == GLFW_KEY_B)
            {
                m_creatingBeam = !m_creatingBeam;
                m_selectedNodeForBeam = -1; // Reset selection
                updateNodeHighlights();
                cout << "Beam creation mode " << (m_creatingBeam ? "enabled" : "disabled") << endl;
                cout << (m_creatingBeam ? "Click two nodes to create a beam" : "") << endl;
            }

            // Delete selected elements
            if (key == GLFW_KEY_DELETE)
            {
                deleteSelectedElements();
            }

            // Clear selections
            if (key == GLFW_KEY_ESCAPE)
            {
                m_selectedNodeForBeam = -1;
                m_creatingBeam = false;
                updateNodeHighlights();
                // Clear the placement mixin selection instead of calling this->clearSelection()
                if (this->placementMixin())
                {
                    this->placementMixin()->clearSelection();
                }
            }
        }

        // Call base implementation
        EnhancedSceneWindow::onKey(key, scancode, action, mods);
    }

private:
    ivf::NodePtr createBeamNode(const glm::vec3 &position)
    {
        auto beamNode = std::make_unique<BeamNode>(m_nextNodeId++, position);
        auto visual = beamNode->visual;

        // Store the beam node
        int nodeId = beamNode->id;
        m_nodes[nodeId] = std::move(beamNode);

        cout << "Created beam node " << nodeId << " at (" << position.x << ", " << position.y << ", " << position.z
             << ")" << endl;

        return visual;
    }

    void selectNodeForOperation(ivf::NodePtr visualNode)
    {
        // Find which beam node this visual belongs to
        int nodeId = findNodeIdByVisual(visualNode);
        if (nodeId == -1)
            return;

        if (m_creatingBeam)
        {
            if (m_selectedNodeForBeam == -1)
            {
                // First node selection
                m_selectedNodeForBeam = nodeId;
                cout << "Selected first node " << nodeId << " for beam creation" << endl;
            }
            else if (m_selectedNodeForBeam != nodeId)
            {
                // Second node selection - create beam
                createBeamElement(m_selectedNodeForBeam, nodeId);
                m_selectedNodeForBeam = -1; // Reset for next beam
            }
        }
        else
        {
            // Regular selection for moving
            m_selectedNodeForBeam = nodeId;
            cout << "Selected node " << nodeId << " for manipulation" << endl;
        }

        updateNodeHighlights();
    }

    void updateNodePosition(ivf::NodePtr visualNode, const glm::vec3 &newPosition)
    {
        int nodeId = findNodeIdByVisual(visualNode);
        if (nodeId == -1)
            return;

        auto &node = m_nodes[nodeId];
        node->position = newPosition;

        // Update all connected beams
        updateConnectedBeams(nodeId);
    }

    void createBeamElement(int nodeIdA, int nodeIdB)
    {
        if (m_nodes.find(nodeIdA) == m_nodes.end() || m_nodes.find(nodeIdB) == m_nodes.end())
        {
            return;
        }

        auto beam = std::make_unique<BeamElement>(m_nextBeamId++, nodeIdA, nodeIdB);

        // Update geometry
        beam->updateGeometry(m_nodes[nodeIdA]->position, m_nodes[nodeIdB]->position);

        // Add to scene
        this->add(beam->visual);

        cout << "Created beam " << beam->id << " connecting nodes " << nodeIdA << " and " << nodeIdB << endl;

        // Store the beam
        int beamId = beam->id;
        m_beams[beamId] = std::move(beam);
    }

    void updateConnectedBeams(int nodeId)
    {
        for (auto &[beamId, beam] : m_beams)
        {
            if (beam->nodeA == nodeId || beam->nodeB == nodeId)
            {
                auto &nodeA = m_nodes[beam->nodeA];
                auto &nodeB = m_nodes[beam->nodeB];
                beam->updateGeometry(nodeA->position, nodeB->position);
            }
        }
    }

    void updateNodeHighlights()
    {
        for (auto &[nodeId, node] : m_nodes)
        {
            if (nodeId == m_selectedNodeForBeam)
            {
                node->visual->setMaterial(m_selectedNodeMaterial);
            }
            else
            {
                node->visual->setMaterial(m_normalNodeMaterial);
            }
        }
    }

    int findNodeIdByVisual(ivf::NodePtr visual)
    {
        for (auto &[nodeId, node] : m_nodes)
        {
            if (node->visual == visual)
            {
                return nodeId;
            }
        }
        return -1;
    }

    void deleteSelectedElements()
    {
        if (m_selectedNodeForBeam != -1)
        {
            // Delete node and connected beams
            deleteNode(m_selectedNodeForBeam);
            m_selectedNodeForBeam = -1;
            updateNodeHighlights();
        }
    }

    void deleteNode(int nodeId)
    {
        if (m_nodes.find(nodeId) == m_nodes.end())
            return;

        // First delete all connected beams
        auto beamIt = m_beams.begin();
        while (beamIt != m_beams.end())
        {
            if (beamIt->second->nodeA == nodeId || beamIt->second->nodeB == nodeId)
            {
                this->remove(beamIt->second->visual);
                beamIt = m_beams.erase(beamIt);
                cout << "Deleted beam " << beamIt->first << endl;
            }
            else
            {
                ++beamIt;
            }
        }

        // Then delete the node
        this->remove(m_nodes[nodeId]->visual);
        m_nodes.erase(nodeId);
        cout << "Deleted node " << nodeId << endl;
    }
};

typedef std::shared_ptr<BeamEditorWindow> BeamEditorWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = BeamEditorWindow::create(1280, 800, "3D Beam Editor");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
