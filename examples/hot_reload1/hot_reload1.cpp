/**
 * @file hot_reload1.cpp
 * @brief Hot reload: a model, a texture and a shader that update when their files change.
 * @ingroup general_examples
 *
 * Demonstrates ResourceWatcher:
 *  - watchModel(): a glTF model reloaded in place (it keeps its animation; onReload gives
 *    its new materials the environment again)
 *  - watchTexture(): an image reloaded into the same texture
 *  - ShaderMaterial::createFromFiles(..., true): shaders recompiled on save
 *  - setOnReload(): a log of what was reloaded
 *
 * The files live in hot_reload_live/ next to the program, created at start. Open them in
 * an editor (the window lists them) and save, or use the buttons, which rewrite them.
 */

#include <imgui.h>
#include <ivf/environment_map.h>
#include <ivf/gl.h>
#include <ivf/gltf_model.h>
#include <ivf/image_io.h>
#include <ivf/logger.h>
#include <ivf/nodes.h>
#include <ivf/pbr_material.h>
#include <ivf/resource_watcher.h>
#include <ivf/shader_material.h>
#include <ivf/skybox.h>
#include <ivfui/ui.h>

#include <array>
#include <cmath>
#include <deque>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;
namespace fs = std::filesystem;

namespace {

const char *vertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
layout(std140) uniform IvfCamera {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 viewPos;
};

out vec3 worldPos;
out vec3 worldNormal;

void main()
{
    vec4 p = model * vec4(aPos, 1.0);
    worldPos = p.xyz;
    worldNormal = normalize(mat3(model) * aNormal);
    gl_Position = projection * view * p;
}
)";

/// Fragment shader: stripes in a colour (edit the colour or the stripes and save)
std::string fragmentShader(const glm::vec3 &color, int stripes)
{
    return R"(#version 330 core
in vec3 worldPos;
in vec3 worldNormal;
out vec4 fragColor;

layout(std140) uniform IvfCamera {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 viewPos;
};

const vec3 color = vec3()" +
           std::to_string(color.r) + ", " + std::to_string(color.g) + ", " + std::to_string(color.b) + R"();
const float stripes = )" + std::to_string(stripes) + R"(.0;

void main()
{
    vec3 n = normalize(worldNormal);
    float light = 0.35 + 0.65 * max(dot(n, normalize(vec3(0.4, 1.0, 0.5))), 0.0);
    float band = step(0.5, fract(worldPos.y * stripes));
    float rim = pow(1.0 - max(dot(n, normalize(viewPos - worldPos)), 0.0), 3.0);
    fragColor = vec4(color * light * mix(0.6, 1.0, band) + rim * 0.4, 1.0);
}
)";
}

/// Texture: tiles in a hue (written as PNG)
bool writeTexture(const std::string &path, float hue)
{
    const int size = 128;
    std::vector<unsigned char> pixels(size * size * 3);
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x)
        {
            const bool tile = ((x / 16) + (y / 16)) % 2 == 0;
            const bool grout = (x % 32) < 2 || (y % 32) < 2;
            const float k = grout ? 0.25f : (tile ? 1.0f : 0.75f);
            for (int c = 0; c < 3; ++c)
            {
                const float v = 0.5f + 0.5f * std::cos(6.2831853f * (hue + float(c) / 3.0f));
                pixels[static_cast<std::size_t>((y * size + x) * 3 + c)] = static_cast<unsigned char>(255.0f * k * v);
            }
        }
    return writePng(path.c_str(), size, size, 3, pixels.data(), size * 3);
}

} // namespace

class HotReloadWindow : public GLFWSceneWindow {
private:
    fs::path m_folder;
    std::string m_modelFile, m_textureFile, m_vertFile, m_fragFile;
    GltfModelPtr m_model;
    EnvironmentMapPtr m_environment;
    std::deque<std::string> m_log;
    float m_hue{0.08f};
    int m_colorIndex{0};
    float m_time{0.0f};

public:
    HotReloadWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<HotReloadWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<HotReloadWindow>(w, h, title);
    }

    void prepareMaterials(GltfModel &model)
    {
        for (auto &material : model.materials())
        {
            material->setEnvironment(m_environment);
            material->setToneMapping(false);
        }
    }

    int onSetup() override
    {
        // The files to edit, created fresh at every start
        m_folder = fs::absolute("hot_reload_live");
        fs::create_directories(m_folder);
        m_modelFile = (m_folder / "model.glb").string();
        m_textureFile = (m_folder / "tiles.png").string();
        m_vertFile = (m_folder / "stripes.vert").string();
        m_fragFile = (m_folder / "stripes.frag").string();
        std::error_code ec;
        fs::copy_file("assets/models/showcase.glb", m_modelFile, fs::copy_options::overwrite_existing, ec);
        if (ec || !writeTexture(m_textureFile, m_hue))
        {
            logError("Could not create the files in " + m_folder.string(), "hot_reload1");
            return -1;
        }
        std::ofstream(m_vertFile) << vertexShader;
        std::ofstream(m_fragFile) << fragmentShader({0.95f, 0.45f, 0.2f}, 6);

        // Lighting: an environment for the glTF materials, a light for the others
        m_environment = EnvironmentMap::create();
        SkySettings sky;
        sky.sunDirection = glm::normalize(glm::vec3(0.5f, 0.6f, 0.6f));
        if (!m_environment->loadSky(sky))
            return -1;
        add(Skybox::create(m_environment->cubemap()));
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection(-sky.sunDirection);
        sun->setDiffuseColor({1.6f, 1.5f, 1.4f});
        sun->setAmbientColor({0.0f, 0.0f, 0.0f});
        lights->apply();

        auto *watcher = renderContext()->resourceWatcher();
        watcher->setOnReload([this](const std::string &name, bool ok) {
            m_log.push_front((ok ? "Reloaded " : "FAILED (old kept) ") + fs::path(name).filename().string());
            if (m_log.size() > 8)
                m_log.pop_back();
        });

        // The model
        auto model = GltfModel::load(m_modelFile);
        if (!model)
            return -1;
        m_model = *model;
        prepareMaterials(*m_model);
        m_model->setAnimation(0);
        m_model->setPos({-1.6f, 0.0f, 0.0f});
        m_model->setScale({0.6f, 0.6f, 0.6f});
        add(m_model);
        watcher->watchModel(m_model, m_modelFile,
                            [this](CompositeNode &node) { prepareMaterials(static_cast<GltfModel &>(node)); });

        // A box with the texture
        auto texture = Texture::create();
        if (!texture->load(m_textureFile))
            return -1;
        watcher->watchTexture(texture);
        auto tiles = PBRMaterial::create();
        tiles->setAlbedoMap(texture); // the reloaded image replaces the old one in the same texture
        tiles->setRoughness(0.4f);
        tiles->setEnvironment(m_environment);
        tiles->setToneMapping(false);
        auto box = Box::create({0.6f, 0.6f, 0.6f});
        box->setPos({1.2f, 0.6f, 0.4f});
        box->setMaterial(tiles);
        add(box);

        // A sphere with the shader (ShaderWatcher, through the same ResourceWatcher)
        auto stripes = ShaderMaterial::createFromFiles(m_vertFile, m_fragFile, "stripes", true);
        if (!stripes)
            return -1;
        auto sphere = Sphere::create(0.6, 48, 24);
        sphere->setPos({2.8f, 0.6f, -0.6f});
        sphere->setMaterial(stripes);
        add(sphere);

        auto concrete = PBRMaterial::create();
        concrete->setAlbedo({0.45f, 0.45f, 0.47f, 1.0f});
        concrete->setRoughness(0.9f);
        concrete->setEnvironment(m_environment);
        concrete->setToneMapping(false);
        auto floor = Box::create({5.0f, 0.02f, 3.5f});
        floor->setPos({0.4f, -0.02f, 0.0f});
        floor->setMaterial(concrete);
        add(floor);

        setToneMapping(ToneMapping::Aces);
        setGamma(2.2f);
        cameraManipulator()->setCameraPosition({0.6f, 3.2f, 6.8f});
        cameraManipulator()->setCameraTarget({0.6f, 0.7f, 0.0f});
        return 0;
    }

    void onUpdate() override
    {
        if (m_model)
            m_model->setTime(static_cast<float>(elapsedTime()));
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({460, 340}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Hot reload");
        ImGui::TextWrapped("Edit and save these files, or use the buttons:");
        ImGui::TextDisabled("%s", m_folder.string().c_str());
        for (const auto *file : {&m_modelFile, &m_textureFile, &m_fragFile, &m_vertFile})
            ImGui::BulletText("%s", fs::path(*file).filename().string().c_str());

        ImGui::Separator();
        if (ImGui::Button("Repaint texture"))
        {
            m_hue = std::fmod(m_hue + 0.23f, 1.0f);
            static_cast<void>(writeTexture(m_textureFile, m_hue)); // a failure shows in the log
        }
        ImGui::SameLine();
        if (ImGui::Button("Recolour shader"))
        {
            static const std::array<glm::vec3, 4> colors{glm::vec3(0.2f, 0.6f, 1.0f), glm::vec3(0.3f, 0.9f, 0.4f),
                                                         glm::vec3(0.9f, 0.3f, 0.8f), glm::vec3(0.95f, 0.45f, 0.2f)};
            m_colorIndex = (m_colorIndex + 1) % static_cast<int>(colors.size());
            std::ofstream(m_fragFile) << fragmentShader(colors[static_cast<std::size_t>(m_colorIndex)], 3 + 3 * m_colorIndex);
        }
        ImGui::SameLine();
        if (ImGui::Button("Break shader"))
            std::ofstream(m_fragFile) << "#version 330 core\nvoid main() { this does not compile }\n";
        ImGui::SameLine();
        if (ImGui::Button("Re-save model"))
        {
            std::error_code ec;
            fs::last_write_time(m_modelFile, fs::file_time_type::clock::now(), ec);
        }

        ImGui::SeparatorText("Reloads");
        if (m_log.empty())
            ImGui::TextDisabled("(none yet)");
        for (const auto &line : m_log)
            ImGui::TextUnformatted(line.c_str());
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = HotReloadWindow::create(1280, 720, "Hot reload");
    app->addWindow(win);
    return app->loop();
}
