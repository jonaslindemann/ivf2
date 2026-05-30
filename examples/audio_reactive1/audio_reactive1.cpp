/**
 * @file audio_reactive1.cpp
 * @brief Audio-reactive visuals — file playback or microphone input.
 *
 * A row of cylinders whose heights map to FFT band energies, and a central
 * sphere that pulses with overall RMS amplitude.
 *
 * Two input modes:
 *   1. File playback  — type a path, click "Play file".
 *      The file plays through the speakers AND drives the visualiser.
 *   2. Microphone     — click "Start mic" to capture live input.
 *
 * Requirements:
 *   - miniaudio declared in vcpkg.json (included via <miniaudio.h>)
 *   - An audio file to play (WAV, OGG, MP3 supported)
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/audio_manager.h>
#include <ivfui/ui.h>

#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;

static constexpr int   DEFAULT_BANDS = 16;
static constexpr float BAR_SPACING   = 1.0f;
static constexpr float BAR_RADIUS    = 0.35f;
static constexpr float MIN_HEIGHT    = 0.1f;
static constexpr float MAX_HEIGHT    = 7.0f;

class AudioReactiveWindow : public GLFWSceneWindow {
    std::vector<std::shared_ptr<CappedCylinder>> m_bars;
    std::shared_ptr<Sphere> m_sphere;
    std::vector<float>      m_smoothBands;
    float                   m_smoothAmp{0.0f};

    enum class Source { None, File, Mic };
    Source m_source{Source::None};

    int   m_bandCount{DEFAULT_BANDS};
    float m_attack{0.7f};    // how fast bars rise  (0 = instant)
    float m_release{0.1f};   // how fast bars fall  (smaller = slower decay)
    float m_gain{5.0f};

    char        m_filePath[512]{"assets/sounds/pf.mp3"};
    bool        m_audioInit{false};
    std::string m_statusMsg;

public:
    AudioReactiveWindow(int w, int h, const std::string& title)
        : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<AudioReactiveWindow> create(int w, int h, const std::string& t)
    { return std::make_shared<AudioReactiveWindow>(w, h, t); }

    int onSetup() override
    {
        cameraManipulator()->setCameraPosition({0, 4, 22});
        enableGrid();

        auto audio = AudioManager::create();
        m_audioInit = audio->init();
        if (!m_audioInit) m_statusMsg = "AudioManager init failed";

        rebuildBars(m_bandCount);

        m_sphere = Sphere::create(1.0f, 24, 24);
        m_sphere->setPos({0.0f, 0.0f, -3.0f});
        auto mat = Material::create();
        mat->setDiffuseColor({0.3f, 0.6f, 1.0f, 1.0f});
        m_sphere->setMaterial(mat);
        add(m_sphere);

        auto lightMgr = LightManager::instance();
        lightMgr->clearLights();
        auto light = lightMgr->addDirectionalLight();
        light->setDirection({-0.5f, -1.0f, -0.5f});
        light->setDiffuseColor({1.2f, 1.2f, 1.2f});
        light->setAmbientColor({0.2f, 0.2f, 0.2f});
        light->setEnabled(true);
        lightMgr->setUseLighting(true);

        return 0;
    }

    void rebuildBars(int bands)
    {
        for (auto& bar : m_bars) remove(bar);
        m_bars.clear();
        m_smoothBands.assign(bands, 0.0f);

        float totalWidth = float(bands - 1) * BAR_SPACING;
        float startX     = -totalWidth * 0.5f;

        for (int i = 0; i < bands; ++i) {
            auto cyl = CappedCylinder::create(BAR_RADIUS, MIN_HEIGHT, 12);
            float hue = float(i) / float(bands);
            auto mat = Material::create();
            mat->setDiffuseColor({hsvToRgb(hue * 360.0f, 0.85f, 1.0f), 1.0f});
            cyl->setMaterial(mat);
            cyl->setPos({startX + float(i) * BAR_SPACING, MIN_HEIGHT * 0.5f, 0.0f});
            add(cyl);
            m_bars.push_back(cyl);
        }
        m_bandCount = bands;
    }

    void stopAll()
    {
        auto audio = AudioManager::instance();
        if (m_source == Source::File) audio->stopFileAnalysis();
        if (m_source == Source::Mic)  audio->stopCapture();
        m_source = Source::None;
        m_statusMsg.clear();
    }

    void startFile()
    {
        stopAll();
        auto audio = AudioManager::instance();
        if (audio->startFileAnalysis(m_filePath, true)) {
            m_source    = Source::File;
            m_statusMsg = std::string("Playing: ") + m_filePath;
        } else {
            m_statusMsg = audio->lastError();
        }
    }

    void startMic()
    {
        stopAll();
        if (AudioManager::instance()->startCapture()) {
            m_source    = Source::Mic;
            m_statusMsg = "Mic active";
        } else {
            m_statusMsg = "Mic capture failed";
        }
    }

    void onUpdate() override
    {
        if (m_source == Source::None) return;

        auto audio = AudioManager::instance();
        auto bands = audio->getFFT(m_bandCount);
        float amp  = audio->getAmplitude();

        // Attack/release smoothing — bars jump up fast, fall slowly
        // This gives a musical "ballistic" feel rather than uniform smoothing
        for (int i = 0; i < m_bandCount; ++i) {
            float target = bands[i] * m_gain;
            float k = (target > m_smoothBands[i]) ? m_attack : m_release;
            m_smoothBands[i] += (target - m_smoothBands[i]) * k;
        }
        float ampTarget = amp * m_gain;
        float ampK = (ampTarget > m_smoothAmp) ? m_attack : m_release;
        m_smoothAmp += (ampTarget - m_smoothAmp) * ampK;

        // Update bar heights
        for (int i = 0; i < int(m_bars.size()); ++i) {
            float h = glm::clamp(m_smoothBands[i] * MAX_HEIGHT, MIN_HEIGHT, MAX_HEIGHT);
            m_bars[i]->setScale({1.0f, h / MIN_HEIGHT, 1.0f});
            m_bars[i]->setPos({m_bars[i]->pos().x, h * 0.5f, 0.0f});
        }

        float s = 1.0f + m_smoothAmp * 1.5f;
        m_sphere->setScale({s, s, s});
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({10, 10}, ImGuiCond_Once);
        ImGui::SetNextWindowSize({360, 420}, ImGuiCond_Once);
        ImGui::Begin("Audio Reactive");

        if (!m_audioInit) {
            ImGui::TextColored({1, 0.3f, 0.3f, 1}, "AudioManager init failed!");
            ImGui::End();
            return;
        }

        ImGui::SeparatorText("Input source");
        ImGui::InputText("File path", m_filePath, sizeof(m_filePath));
        if (ImGui::Button("Play file")) startFile();
        ImGui::SameLine();
        if (ImGui::Button("Stop##f")) stopAll();

        ImGui::Spacing();
        if (ImGui::Button("Start mic")) startMic();
        ImGui::SameLine();
        if (ImGui::Button("Stop##m")) stopAll();

        if (!m_statusMsg.empty()) {
            ImGui::Spacing();
            ImGui::TextWrapped("%s", m_statusMsg.c_str());
        }

        ImGui::SeparatorText("Controls");
        ImGui::SliderFloat("Gain",    &m_gain,    1.0f, 20.0f);
        ImGui::SliderFloat("Attack",  &m_attack,  0.01f, 1.0f);
        ImGui::SliderFloat("Release", &m_release, 0.01f, 0.5f);

        int newBands = m_bandCount;
        if (ImGui::SliderInt("Bands", &newBands, 4, 32) && newBands != m_bandCount)
            rebuildBars(newBands);

        ImGui::SeparatorText("Meters");
        ImGui::Text("Amplitude: %.4f", m_smoothAmp);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p  = ImGui::GetCursorScreenPos();
        ImVec2 sz = {ImGui::GetContentRegionAvail().x, 12};
        dl->AddRectFilled(p, {p.x + sz.x, p.y + sz.y}, IM_COL32(50,50,50,255));
        float fill = glm::clamp(m_smoothAmp / (m_gain > 0 ? m_gain : 1.0f), 0.0f, 1.0f);
        dl->AddRectFilled(p, {p.x + sz.x * fill, p.y + sz.y}, IM_COL32(80, 200, 100, 255));
        ImGui::Dummy(sz);

        ImGui::Spacing();
        ImGui::Text("FFT:");
        for (int i = 0; i < m_bandCount; ++i) {
            float v = glm::clamp(m_smoothBands[i] / (m_gain > 0 ? m_gain : 1.0f) * 5.0f, 0.0f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                ImVec4(float(i)/m_bandCount, 1.0f - float(i)/m_bandCount, 0.5f, 1.0f));
            ImGui::ProgressBar(v, {-1, 7});
            ImGui::PopStyleColor();
        }

        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_SAMPLES, 4);
    auto win = AudioReactiveWindow::create(1280, 800, "Audio Reactive — audio_reactive1");
    app->addWindow(win);
    return app->loop();
}
