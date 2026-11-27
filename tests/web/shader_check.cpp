// Compiles every built-in shader of ivf2 in the current OpenGL context and reports failures.
//
// Built in web builds (preset wasm): in a browser the shaders go through the GLSL ES
// translation (ivf/glsl_es.h) and WebGL 2's compiler, which is stricter than desktop drivers
// (no implicit int-to-float conversion, constant sampler-array indices, ...). The result is
// logged and put in the page title: "shader_check: <n> failed of <m>".

#include <ivf/bump_shaders.h>
#include <ivf/fragment_shader.h>
#include <ivf/logger.h>
#include <ivf/pbr_shaders.h>
#include <ivf/post_shaders.h>
#include <ivf/procedural_shaders.h>
#include <ivf/screen_space_shaders.h>
#include <ivf/shadow_shaders.h>
#include <ivf/stock_shaders.h>
#include <ivf/vertex_shader.h>
#include <ivfui/glfw_application.h>
#include <ivfui/glfw_window.h>

#include <filesystem>
#include <string>
#include <vector>

namespace {

struct Source {
    std::string name;
    bool vertex;
    std::string code;
};

std::vector<Source> builtinShaders()
{
    using namespace ivf;
    std::vector<Source> s = {
        {"basic.vert", true, basic_vert_shader_source},
        {"basic.frag", false, basic_frag_shader_source},
        {"bump.vert", true, bump_vert_shader_source},
        {"bump.frag", false, bump_frag_shader_source},
        {"pbr.vert", true, pbr_vert_shader_source},
        {"pbr.frag", false, pbr_frag_shader_source},
        {"render_to_texture.vert", true, render_to_texture_vert_shader_source_330},
        {"render_to_texture.frag", false, render_to_texture_frag_shader_source_330},
        {"shadow_depth.vert", true, shadow_depth_vert_shader},
        {"shadow_depth.frag", false, shadow_depth_frag_shader},
        {"proc_quad.vert", true, proc_quad_vert_shader},
        {"proc_checkerboard.frag", false, proc_checkerboard_frag_shader},
        {"proc_noise.frag", false, proc_noise_frag_shader},
        {"proc_gradient.frag", false, proc_gradient_frag_shader},
        {"ssao.frag", false, ssao_frag_shader_source},
        {"ssao_apply.frag", false, ssao_apply_frag_shader_source},
        {"ssr.frag", false, ssr_frag_shader_source},
    };
    const std::vector<std::pair<const char *, const std::string *>> post = {
        {"vignette", &vignette_frag_shader_source},
        {"chromatic", &chromatic_frag_shader_source},
        {"filmgrain", &filmgrain_frag_shader_source},
        {"blur", &blur_frag_shader_source},
        {"tint", &tint_frag_shader_source},
        {"fade", &fade_frag_shader_source},
        {"bloom", &bloom_frag_shader_source},
        {"dither", &dither_frag_shader_source},
        {"pixelate", &pixelate_frag_shader_source},
        {"edge_detect", &edge_detect_frag_shader_source},
        {"wave_distortion", &wave_distortion_frag_shader_source},
        {"swirl", &swirl_frag_shader_source},
        {"kaleidoscope", &kaleidoscope_frag_shader_source},
        {"glitch", &glitch_frag_shader_source},
        {"scanline", &scanline_frag_shader_source},
        {"posterize", &posterize_frag_shader_source},
        {"color_grading", &color_grading_frag_shader_source},
        {"night_vision", &night_vision_frag_shader_source},
        {"feedback", &feedback_frag_shader_source},
        {"motion_blur", &motion_blur_frag_shader_source},
        {"halftone", &halftone_frag_shader_source},
        {"fog", &fog_frag_shader_source},
        {"dof_distance", &dof_distance_frag_shader_source},
        {"dof", &dof_frag_shader_source},
    };
    for (const auto &[name, code] : post)
        s.push_back({std::string(name) + ".frag", false, *code});
    return s;
}

// Shader files shipped in data/shaders (preloaded as shaders/)
std::vector<Source> fileShaders()
{
    std::vector<Source> s;
    std::error_code ec;
    for (const auto &entry : std::filesystem::directory_iterator("shaders", ec))
    {
        const std::string name = entry.path().filename().string();
        const bool vertex = name.ends_with(".vert") || name.ends_with(".vs.glsl") || name.ends_with(".vs");
        const bool fragment = name.ends_with(".frag") || name.ends_with(".fs.glsl") || name.ends_with(".fs");
        if (!vertex && !fragment)
            continue;
        auto shader = ivf::Shader::create(entry.path().string()); // reads the file
        s.push_back({"shaders/" + name, vertex, shader->source()});
    }
    return s;
}

class CheckWindow : public ivfui::GLFWWindow {
public:
    using GLFWWindow::GLFWWindow;

    int onSetup() override
    {
        auto sources = builtinShaders();
        auto files = fileShaders();
        sources.insert(sources.end(), files.begin(), files.end());

        int failed = 0;
        for (const auto &src : sources)
        {
            bool ok = false;
            if (src.vertex)
            {
                auto shader = ivf::VertexShader::create();
                shader->setSource(src.code);
                ok = shader->compile();
            }
            else
            {
                auto shader = ivf::FragmentShader::create();
                shader->setSource(src.code);
                ok = shader->compile();
            }
            if (!ok)
            {
                ++failed;
                ivf::logError("FAILED: " + src.name, "ShaderCheck");
            }
        }

        const std::string summary =
            "shader_check: " + std::to_string(failed) + " failed of " + std::to_string(sources.size());
        if (failed == 0)
            ivf::logInfo(summary, "ShaderCheck");
        else
            ivf::logError(summary, "ShaderCheck");
        glfwSetWindowTitle(ref(), summary.c_str());
        return 0;
    }

    void onDraw() override
    {
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};

} // namespace

int main()
{
    auto app = ivfui::GLFWApplication::create();
    auto window = std::make_shared<CheckWindow>(640, 360, "shader_check");
    app->addWindow(window);
    return app->loop();
}
