// Desktop GLSL -> GLSL ES 3.00 translation (ivf/glsl_es.h), used by web builds (WebGL 2).

#include "test_main.h"

#include <ivf/bump_shaders.h>
#include <ivf/glsl_es.h>
#include <ivf/pbr_shaders.h>
#include <ivf/post_shaders.h>
#include <ivf/procedural_shaders.h>
#include <ivf/render_context.h>
#include <ivf/screen_space_shaders.h>
#include <ivf/shadow_shaders.h>
#include <ivf/stock_shaders.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using namespace ivf;

namespace {

const UniformDefault *findDefault(const GlslEsTranslation &t, const std::string &name)
{
    for (const auto &d : t.defaults)
        if (d.name == name)
            return &d;
    return nullptr;
}

std::size_t lineCount(const std::string &s)
{
    return static_cast<std::size_t>(std::count(s.begin(), s.end(), '\n'));
}

// Lines after the inserted "#line n" directive
std::string bodyAfterHeader(const std::string &translated)
{
    const auto pos = translated.find("#line ");
    return translated.substr(translated.find('\n', pos) + 1);
}

} // namespace

IVF_TEST(version_line_becomes_es_with_precisions)
{
    const std::string src = "#version 330 core\nin vec3 aPos;\nvoid main() { gl_Position = vec4(aPos, 1.0); }\n";
    const auto t = translateToGlslEs(src);
    IVF_CHECK(t.source.starts_with("#version 300 es\n"));
    IVF_CHECK(t.source.find("precision highp float;") != std::string::npos);
    IVF_CHECK(t.source.find("precision highp sampler2DShadow;") != std::string::npos);
    IVF_CHECK(t.source.find("#version 330") == std::string::npos);
    // The next line keeps its number (2) for compiler messages
    IVF_CHECK(t.source.find("#line 2\nin vec3 aPos;") != std::string::npos);
    IVF_CHECK(t.defaults.empty() && t.warnings.empty());
}

IVF_TEST(leading_blank_lines_and_missing_version)
{
    // WebGL wants #version on the first line: what precedes it is dropped, #line keeps numbers
    const auto t = translateToGlslEs("\n// comment\n#version 410 core\nvoid main() {}\n");
    IVF_CHECK(t.source.starts_with("#version 300 es\n"));
    IVF_CHECK(t.source.find("#line 4\nvoid main") != std::string::npos);

    const auto u = translateToGlslEs("void main() {}\n");
    IVF_CHECK(u.source.starts_with("#version 300 es\n"));
    IVF_CHECK(u.source.find("#line 1\nvoid main") != std::string::npos);
}

IVF_TEST(uniform_initialisers_are_removed_and_evaluated)
{
    const std::string src = "#version 330 core\n"
                            "#define MODE_B 2\n"
                            "uniform bool useLighting = true;\n"
                            "uniform int mode = MODE_B;\n"
                            "uniform float gamma = 2.2f;\n"
                            "uniform highp float neg = -0.5;\n"
                            "uniform vec3 up = vec3(0.0, 1.0, 0.0);\n"
                            "uniform vec3 grey = vec3(0.7);\n"
                            "uniform vec4 c = vec4(vec3(1.0, 2.0, 3.0), 4.0);\n"
                            "uniform float m[4] = float[](\n"
                            "    1.0, 2.0,\n"
                            "    3.0, 4.0);\n"
                            "uniform float plain;\n"
                            "uniform float sum = 1.0/4.0 + 2.0*0.5 - (0.5 - 0.25);\n"
                            "uniform vec2 half2 = vec2(1.0, 3.0) * 0.5;\n"
                            "void main() {}\n";
    const auto t = translateToGlslEs(src);
    IVF_CHECK(t.warnings.empty());
    IVF_CHECK(t.defaults.size() == 10);

    // No initialisers left, and the same number of lines (line numbers in messages stay right)
    IVF_CHECK(t.source.find("= true") == std::string::npos);
    IVF_CHECK(t.source.find("float[]") == std::string::npos);
    IVF_CHECK(t.source.find("uniform float gamma;") != std::string::npos);
    IVF_CHECK(t.source.find("uniform float plain;") != std::string::npos);
    IVF_CHECK(lineCount(bodyAfterHeader(t.source)) == lineCount(src) - 1);

    auto check = [&](const char *name, std::vector<double> values, int arraySize = 0) {
        const auto *d = findDefault(t, name);
        IVF_CHECK(d != nullptr);
        if (d == nullptr)
            return;
        IVF_CHECK(d->arraySize == arraySize);
        IVF_CHECK(d->values.size() == values.size());
        for (std::size_t i = 0; i < std::min(values.size(), d->values.size()); ++i)
            IVF_CHECK_NEAR(d->values[i], values[i], 1e-9);
    };
    check("useLighting", {1.0});
    check("mode", {2.0});
    check("gamma", {2.2});
    check("neg", {-0.5});
    check("up", {0.0, 1.0, 0.0});
    check("grey", {0.7, 0.7, 0.7});
    check("c", {1.0, 2.0, 3.0, 4.0});
    check("m", {1.0, 2.0, 3.0, 4.0}, 4);
    check("sum", {1.0});
    check("half2", {0.5, 1.5});
    IVF_CHECK(findDefault(t, "useLighting")->type == "bool");
    IVF_CHECK(findDefault(t, "up")->type == "vec3");
}

IVF_TEST(comments_and_blocks_are_left_alone)
{
    const std::string src = "#version 330 core\n"
                            "// uniform float commented = 1.0; still a comment\n"
                            "/* uniform int alsoCommented = 2;\n"
                            "   more */\n"
                            "uniform IvfCamera {\n"
                            "    mat4 view;\n"
                            "};\n"
                            "uniform sampler2D tex; // uniform float x = 3.0;\n"
                            "float nonUniform = 1.0;\n"
                            "void main() {}\n";
    const auto t = translateToGlslEs(src);
    IVF_CHECK(t.defaults.empty());
    IVF_CHECK(t.warnings.empty());
    IVF_CHECK(t.source.find("// uniform float commented = 1.0; still a comment") != std::string::npos);
    IVF_CHECK(t.source.find("uniform IvfCamera {") != std::string::npos);
    IVF_CHECK(t.source.find("float nonUniform = 1.0;") != std::string::npos);
}

IVF_TEST(unsupported_initialisers_are_reported)
{
    const auto t = translateToGlslEs("#version 330 core\nuniform mat4 m = mat4(1.0);\nuniform float f = sin(1.0);\n");
    IVF_CHECK(t.defaults.empty());
    IVF_CHECK(t.warnings.size() == 2);
    IVF_CHECK(t.source.find("uniform mat4 m;") != std::string::npos);
    IVF_CHECK(t.source.find("uniform float f;") != std::string::npos);
}

IVF_TEST(desktop_gl_keeps_sources_unchanged)
{
    IVF_CHECK(!RenderContext::isGles()); // no GL loaded in the tests
    const std::string src = "#version 330 core\nuniform float gamma = 2.2;\nvoid main() {}\n";
    std::vector<UniformDefault> defaults(1);
    IVF_CHECK(prepareShaderSource(src, &defaults) == src);
    IVF_CHECK(defaults.empty());
}

IVF_TEST(builtin_shaders_translate_without_warnings)
{
    const std::vector<std::pair<const char *, const std::string *>> shaders = {
        {"basic_vert", &basic_vert_shader_source},
        {"basic_frag", &basic_frag_shader_source},
        {"bump_vert", &bump_vert_shader_source},
        {"bump_frag", &bump_frag_shader_source},
        {"pbr_vert", &pbr_vert_shader_source},
        {"pbr_frag", &pbr_frag_shader_source},
        {"render_to_texture_vert", &render_to_texture_vert_shader_source_330},
        {"render_to_texture_frag", &render_to_texture_frag_shader_source_330},
        {"dither_frag", &dither_frag_shader_source},
        {"fog_frag", &fog_frag_shader_source},
        {"dof_frag", &dof_frag_shader_source},
        {"proc_checkerboard", &proc_checkerboard_frag_shader},
        {"proc_noise", &proc_noise_frag_shader},
        {"proc_gradient", &proc_gradient_frag_shader},
        {"ssao", &ssao_frag_shader_source},
        {"ssao_apply", &ssao_apply_frag_shader_source},
        {"ssr", &ssr_frag_shader_source},
        {"shadow_depth_vert", &shadow_depth_vert_shader},
        {"shadow_depth_frag", &shadow_depth_frag_shader},
    };
    for (const auto &[name, source] : shaders)
    {
        const auto t = translateToGlslEs(*source);
        if (!t.warnings.empty())
            std::printf("  %s: %s\n", name, t.warnings.front().c_str());
        IVF_CHECK(t.warnings.empty());
        IVF_CHECK(t.source.starts_with("#version 300 es\n"));
        // Translating again finds nothing left to remove
        IVF_CHECK(translateToGlslEs(t.source).defaults.empty());
    }

    // Spot checks of values that differ from GLSL ES's zero default
    const auto post = translateToGlslEs(render_to_texture_frag_shader_source_330);
    IVF_CHECK(findDefault(post, "exposure") != nullptr && findDefault(post, "exposure")->values[0] == 1.0);
    const auto dither = translateToGlslEs(dither_frag_shader_source);
    IVF_CHECK(findDefault(dither, "ditherMatrix") != nullptr && findDefault(dither, "ditherMatrix")->arraySize == 16);
    const auto stock = translateToGlslEs(basic_frag_shader_source);
    IVF_CHECK(findDefault(stock, "blendMode") != nullptr && findDefault(stock, "blendMode")->values[0] == 1.0);
}

IVF_TEST_MAIN()
