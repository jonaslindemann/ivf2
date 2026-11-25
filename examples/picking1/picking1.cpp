/**
 * @file picking1.cpp
 * @brief CPU picking: ray casts against the scene's triangles.
 * @ingroup general_examples
 *
 * Demonstrates ivf::pick() and GLFWSceneWindow::pick():
 *  - the node, point, normal and triangle under the mouse, with each mesh's bounding volume
 *    hierarchy (Mesh::bvh()) keeping ray casts fast even for a 65 000 triangle sphere
 *  - picking instanced copies, and animated meshes (a skinned tentacle and a morphing blob
 *    of a glTF model) where they currently are
 *  - SelectionMethod::Ray: hover events (onEnterNode/onLeaveNode) without the selection buffer
 *  - Ctrl + click drops a marker on the surface; at start a grid of rays marks the scene
 */

#include <imgui.h>
#include <ivf/environment_map.h>
#include <ivf/gl.h>
#include <ivf/gltf_model.h>
#include <ivf/mesh_bvh.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <array>
#include <string>
#include <unordered_set>
#include <vector>

using namespace ivf;
using namespace ivfui;

class PickingWindow : public GLFWSceneWindow {
private:
    CompositeNodePtr m_markers;
    MaterialPtr m_markerMaterial;
    MaterialPtr m_highlight;
    MaterialPtr m_hoveredMaterial;
    Node *m_hovered{nullptr};
    GltfModelPtr m_model;
    RayHit m_hit;
    std::vector<std::pair<glm::vec3, glm::vec3>> m_normals; ///< Marker positions and normals.
    std::unordered_set<const Node *> m_markerNodes;
    bool m_scattered{false};
    int m_method{1};

public:
    PickingWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<PickingWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<PickingWindow>(w, h, title);
    }

    static MaterialPtr material(glm::vec3 color)
    {
        auto m = Material::create();
        m->setDiffuseColor(glm::vec4(color, 1.0f));
        m->setAmbientColor(glm::vec4(color * 0.4f, 1.0f));
        m->setSpecularColor({0.4f, 0.4f, 0.4f, 1.0f});
        return m;
    }

    void addMarker(const RayHit &hit)
    {
        auto marker = Sphere::create(0.06, 12, 6);
        marker->setPos(hit.position);
        marker->setMaterial(m_markerMaterial);
        marker->setCastShadows(false);
        m_markers->add(marker);
        m_markerNodes.insert(marker.get());
        m_normals.emplace_back(hit.position, hit.normal);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.4f, -1.0f, -0.5f});
        sun->setDiffuseColor({0.9f, 0.9f, 0.85f});
        sun->setAmbientColor({0.35f, 0.35f, 0.4f});
        lights->apply();

        m_highlight = material({1.0f, 0.85f, 0.2f});
        m_markerMaterial = material({1.0f, 0.15f, 0.1f});

        auto floor = Box::create({7.0f, 0.05f, 4.0f});
        floor->setPos({0.0f, -0.05f, 0.0f});
        floor->setMaterial(material({0.5f, 0.52f, 0.55f}));
        floor->setName("Floor");
        add(floor);

        // Primitives
        const std::array<glm::vec3, 4> colors{glm::vec3(0.3f, 0.55f, 0.9f), glm::vec3(0.4f, 0.8f, 0.45f),
                                              glm::vec3(0.85f, 0.4f, 0.3f), glm::vec3(0.75f, 0.5f, 0.85f)};
        auto cube = Box::create({0.4f, 0.4f, 0.4f});
        cube->setPos({-4.5f, 0.4f, 1.2f});
        cube->setEulerAngles(0.0f, 30.0f, 0.0f);
        auto cone = Cone::create(0.5, 1.2, 32, 1);
        cone->setPos({-3.2f, 0.6f, 1.6f});
        auto capsule = Capsule::create(0.3, 0.6, 32, 4);
        capsule->setPos({-1.9f, 0.6f, 1.4f});
        capsule->setEulerAngles(0.0f, 0.0f, 90.0f);
        auto torus = Dodecahedron::create(0.5);
        torus->setPos({-4.0f, 0.5f, -0.4f});
        int i = 0;
        for (auto node : std::vector<TransformNodePtr>{cube, cone, capsule, torus})
        {
            node->setMaterial(material(colors[static_cast<std::size_t>(i++ % 4)]));
            add(node);
        }
        cube->setName("Box");
        cone->setName("Cone");
        capsule->setName("Capsule");
        torus->setName("Dodecahedron");

        // A dense mesh: the BVH keeps ray casts fast
        auto dense = Sphere::create(0.9, 256, 128);
        dense->setPos({-1.6f, 0.9f, -1.5f});
        dense->setMaterial(material({0.85f, 0.85f, 0.8f}));
        dense->setName("Sphere (65k triangles)");
        add(dense);

        // Instanced copies of one cylinder
        auto pillar = Cylinder::create(0.15, 1.4, 24, 1);
        pillar->setMaterial(material({0.6f, 0.6f, 0.65f}));
        for (int k = 0; k < 6; ++k)
        {
            auto copy = InstanceNode::create();
            copy->setNode(pillar);
            copy->setPos({-4.6f + 0.55f * float(k), 0.7f, -2.6f});
            copy->setName("Pillar " + std::to_string(k + 1));
            add(copy);
        }

        // An animated glTF model: picks follow the skinned and morphed meshes
        auto model = GltfModel::load("assets/models/showcase.glb");
        if (!model)
            return -1;
        m_model = *model;
        auto environment = EnvironmentMap::create();
        if (!environment->loadSky(SkySettings{}))
            return -1;
        for (auto &material : m_model->materials())
            material->setEnvironment(environment);
        m_model->setAnimation(0);
        m_model->setPos({2.8f, 0.0f, -0.2f});
        m_model->setScale({0.75f, 0.75f, 0.75f});
        add(m_model);

        m_markers = CompositeNode::create();
        add(m_markers);

        setSelectionEnabled(true);
        setSelectionMethod(SelectionMethod::Ray);

        cameraManipulator()->setCameraPosition({0.0f, 5.5f, 8.5f});
        cameraManipulator()->setCameraTarget({0.0f, 0.4f, -0.3f});
        return 0;
    }

    void onUpdate() override
    {
        if (m_model)
            m_model->setTime(static_cast<float>(elapsedTime()));

        // Once the scene has been drawn: mark it with a grid of rays through the window
        if (!m_scattered && frameCount() >= 2)
        {
            m_scattered = true;
            int w = 0, h = 0;
            glfwGetWindowSize(ref(), &w, &h);
            for (int y = 0; y < 9; ++y)
                for (int x = 0; x < 16; ++x)
                    if (auto hit = pick((x + 0.5) * w / 16.0, (y + 0.5) * h / 9.0); hit && hit.node->name() != "Floor")
                        addMarker(hit);
        }

        // What is under the mouse
        PickOptions options;
        options.skip = [this](const DrawItem &item) { return m_markerNodes.count(item.node) > 0; };
        const bool hovered = glfwGetWindowAttrib(ref(), GLFW_HOVERED) == GLFW_TRUE;
        m_hit = hovered ? pick(mouseX(), mouseY(), options) : RayHit{};

        auto *debug = renderContext()->debugDraw();
        for (const auto &[position, normal] : m_normals)
            debug->arrow(position, normal * 0.35f, {1.0f, 0.3f, 0.2f, 1.0f});
        if (m_hit)
            debug->arrow(m_hit.position, m_hit.normal * 0.6f, {0.2f, 1.0f, 0.4f, 1.0f});
    }

    void onEnterNode(Node *node) override
    {
        // Hover highlight (nodes whose meshes have their own materials, like the glTF model, keep them)
        m_hovered = node;
        m_hoveredMaterial = node->material();
        node->setMaterial(m_highlight);
    }

    void onLeaveNode(Node *node) override
    {
        if (node == m_hovered)
        {
            node->setMaterial(m_hoveredMaterial);
            m_hovered = nullptr;
            m_hoveredMaterial = nullptr;
        }
    }

    void onMouseButton(int button, int action, int mods) override
    {
        // Ctrl + click: a marker on the surface
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL) && m_hit)
            addMarker(m_hit);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({380, 300}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Picking");
        if (ImGui::RadioButton("Selection buffer", &m_method, 0) || ImGui::RadioButton("Ray casting", &m_method, 1))
            setSelectionMethod(m_method == 0 ? SelectionMethod::Buffer : SelectionMethod::Ray);
        ImGui::TextDisabled("Hover events use %s", m_method == 0 ? "the selection buffer" : "ray casting");

        ImGui::SeparatorText("Under the mouse");
        if (m_hit)
        {
            ImGui::Text("%s", m_hit.node->name().empty() ? "(unnamed node)" : m_hit.node->name().c_str());
            ImGui::Text("Distance  %.3f", m_hit.distance);
            ImGui::Text("Position  %.2f, %.2f, %.2f", m_hit.position.x, m_hit.position.y, m_hit.position.z);
            ImGui::Text("Normal    %.2f, %.2f, %.2f", m_hit.normal.x, m_hit.normal.y, m_hit.normal.z);
            if (m_hit.mesh != nullptr)
            {
                ImGui::Text("Triangle  %d", m_hit.triangle);
                if (auto bvh = m_hit.mesh->bvh())
                    ImGui::TextDisabled("BVH: %zu triangles, %zu boxes, depth %d", bvh->triangleCount(),
                                        bvh->nodeCount(), bvh->depth());
            }
            else
                ImGui::TextDisabled("(bounding box only)");
        }
        else
            ImGui::TextDisabled("nothing");

        ImGui::SeparatorText("Markers");
        ImGui::Text("%zu markers (Ctrl + click adds one)", m_normals.size());
        if (ImGui::Button("Clear"))
        {
            m_markers->clear();
            m_markerNodes.clear();
            m_normals.clear();
        }
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = PickingWindow::create(1280, 720, "CPU picking");
    app->addWindow(win);
    return app->loop();
}
