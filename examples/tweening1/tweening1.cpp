/**
 * @file tweening1.cpp
 * @brief Tween system example — animated positions with easing curves.
 * @ingroup tween_examples
 *
 * Click any shape to animate it to a random position with the selected
 * easing curve. Shows TweenManager, Easing enum, and timer callbacks.
 *
 * Also demonstrates TimerManager::every() for a periodic re-shuffle.
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <random>
#include <string>

using namespace ivf;
using namespace ivfui;

static std::mt19937 g_rng{std::random_device{}()};
static float randF(float lo, float hi)
{
    std::uniform_real_distribution<float> d(lo, hi);
    return d(g_rng);
}

class TweenWindow : public GLFWSceneWindow {
private:
    std::vector<TransformNodePtr> m_shapes;
    std::vector<glm::vec3>        m_positions; // current target
    int    m_easingIdx{7};  // default: ElasticOut
    bool   m_autoShuffle{false};

    static constexpr const char* k_easingNames[] = {
        "Linear","QuadIn","QuadOut","QuadInOut",
        "CubicIn","CubicOut","CubicInOut",
        "ElasticOut","BounceOut","BackOut",
        "SineIn","SineOut","SineInOut"
    };

public:
    TweenWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<TweenWindow> create(int w, int h, std::string t)
    { return std::make_shared<TweenWindow>(w, h, t); }

    glm::vec3 randomPos()
    { return { randF(-6, 6), randF(0, 4), randF(-6, 6) }; }

    void tweenShape(int idx, glm::vec3 target)
    {
        auto easing = static_cast<Easing>(m_easingIdx);
        // Tween the stored position; onUpdate() applies it to the node.
        TweenManager::instance()->to(m_positions[idx], target, 1.2f, easing);
    }

    void shuffleAll()
    {
        for (size_t i = 0; i < m_shapes.size(); ++i)
            tweenShape(int(i), randomPos());
    }

    int onSetup() override
    {
        setSelectionEnabled(true);

        const glm::vec4 colors[] = {
            {1,0.3f,0.3f,1}, {0.3f,1,0.4f,1}, {0.3f,0.6f,1,1},
            {1,0.9f,0.2f,1}, {0.9f,0.3f,1,1}, {0.3f,1,0.9f,1},
        };

        for (int i = 0; i < 6; ++i) {
            TransformNodePtr node;
            if (i % 3 == 0)      node = Sphere::create(0.5f);
            else if (i % 3 == 1) node = Box::create({0.8f, 0.8f, 0.8f});
            else                 node = CappedCylinder::create(0.4f, 0.8f);

            auto mat = Material::create();
            mat->setDiffuseColor(colors[i]);
            node->setMaterial(mat);

            glm::vec3 p = randomPos();
            node->setPos(p);
            m_positions.push_back(p);
            m_shapes.push_back(node);
            add(node);

            // Click to individually tween (capture index i)
            int idx = i;
            node->addBehavior(std::make_shared<ClickCallbackBehavior>([this, idx]{
                tweenShape(idx, randomPos());
            }));
        }

        enableAxis();
        enableGrid();
        cameraManipulator()->setCameraPosition({0, 6, 14});

        // Auto-shuffle timer (starts disabled)
        TimerManager::instance()->every(2.0f, [this]{
            if (m_autoShuffle) shuffleAll();
        });

        return 0;
    }

    void onUpdate() override
    {
        for (size_t i = 0; i < m_shapes.size(); ++i)
            m_shapes[i]->setPos(m_positions[i]);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({280, 200}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Tweening");
        ImGui::Text("Click any shape to tween it.");
        ImGui::Combo("Easing", &m_easingIdx, k_easingNames, 13);
        if (ImGui::Button("Shuffle All")) shuffleAll();
        ImGui::SameLine();
        ImGui::Checkbox("Auto shuffle", &m_autoShuffle);
        ImGui::Separator();
        ImGui::Text("Active tweens: %d", TweenManager::instance()->count());
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto win = TweenWindow::create(1024, 768, "Tweening");
    win->maximize();
    app->addWindow(win);
    return app->loop();
}
