#include <ivf/shader_manager.h>
#include <ivf/render_context.h>

#include <ivf/fragment_shader.h>
#include <ivf/program.h>
#include <ivf/stock_shaders.h>
#include <ivf/pbr_shaders.h>
#include <ivf/bump_shaders.h>
#include <ivf/post_shaders.h>
#include <ivf/vertex_shader.h>
#include <ivf/logger.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace ivf;
using namespace std;

// The manager lives in the current RenderContext (see ivf/render_context.h).
ShaderManager *ShaderManager::instance()
{
    return RenderContext::currentOrFallback()->shaders();
}

ShaderManager *ShaderManager::create()
{
    return instance();
}

void ShaderManager::drop()
{
    // Managers are released together with their RenderContext.
}


namespace {

constexpr GLuint kDefaultCubemapUnit = 15;

void bindDefaultTexture2D(GLuint unit)
{
    // The fallback texture belongs to the current RenderContext (one per GL context).
    const GLuint texture = RenderContext::currentOrFallback()->fallbackTexture2D();
    GLint activeTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(activeTexture);
}

void bindDefaultCubemap(GLuint unit)
{
    // The fallback texture belongs to the current RenderContext (one per GL context).
    const GLuint texture = RenderContext::currentOrFallback()->fallbackCubemap();
    GLint activeTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glActiveTexture(activeTexture);
}

}

ShaderManager::ShaderManager()
    : m_currentProgram(nullptr), m_linkErrors(false), m_vertexCompileErrors(false), m_fragCompileErrors(false)
{
    logDebug("ShaderManager Constructor", "ShaderManager");
}

ShaderManager::~ShaderManager()
{
    logDebug("ShaderManager Destructor", "ShaderManager");
    // Clean up any remaining programs
    m_programs.clear();
}




Result<ProgramPtr> ShaderManager::buildProgram(std::shared_ptr<VertexShader> vertexShader,
                                               std::shared_ptr<FragmentShader> fragmentShader,
                                               const std::string &name, bool makeCurrent)
{
    // The shader and program classes log the driver output themselves; here we add one line
    // that says which program failed and return the details in the Error for the caller.
    auto fail = [&](ErrorCode code, const std::string &what, const std::string &infoLog) -> Result<ProgramPtr> {
        const std::string summary = "Shader program '" + name + "': " + what;
        logError(summary, "ShaderManager");
        return silentError(code, infoLog.empty() ? summary : summary + ":\n" + infoLog);
    };

    m_vertexCompileErrors = !vertexShader->compile();
    m_fragCompileErrors = !fragmentShader->compile();
    m_linkErrors = false;

    if (m_vertexCompileErrors)
        return fail(ErrorCode::CompileFailed, "vertex shader failed to compile", vertexShader->infoLog());
    if (m_fragCompileErrors)
        return fail(ErrorCode::CompileFailed, "fragment shader failed to compile", fragmentShader->infoLog());

    auto program = std::make_shared<Program>();
    program->setName(name);
    program->addShader(vertexShader);
    program->addShader(fragmentShader);

    m_linkErrors = !program->link();
    if (m_linkErrors)
        return fail(ErrorCode::LinkFailed, "failed to link", program->infoLog());

    program->bindAttribLoc(0, "aPos");
    program->bindAttribLoc(1, "aColor");
    program->bindAttribLoc(2, "aTex");
    program->bindAttribLoc(3, "aNormal");

    m_programs[name] = program;

    if (makeCurrent)
    {
        program->use();
        m_currentProgram = program;
    }

    program->printAttribs();

    return program;
}

Result<ProgramPtr> ShaderManager::compileProgramFromFiles(const std::string &vertexFile,
                                                          const std::string &fragmentFile, const std::string &name,
                                                          bool makeCurrent)
{
    logInfo("Loading shader program '" + name + "' from " + vertexFile + " and " + fragmentFile, "ShaderManager");

    auto readShader = [&](Shader &shader, const std::string &file) -> Result<void> {
        std::error_code ec;
        if (!std::filesystem::exists(file, ec))
            return makeError(ErrorCode::FileNotFound, "Shader program '" + name + "': file '" + file + "' not found",
                             "ShaderManager");
        shader.setFilename(file);
        if (!shader.read())
            return makeError(ErrorCode::ReadFailed, "Shader program '" + name + "': could not read '" + file + "'",
                             "ShaderManager");
        return {};
    };

    auto vertexShader = std::make_shared<VertexShader>();
    if (auto ok = readShader(*vertexShader, vertexFile); !ok)
        return std::unexpected(ok.error());

    auto fragmentShader = std::make_shared<FragmentShader>();
    if (auto ok = readShader(*fragmentShader, fragmentFile); !ok)
        return std::unexpected(ok.error());

    return buildProgram(vertexShader, fragmentShader, name, makeCurrent);
}

Result<ProgramPtr> ShaderManager::compileProgramFromStrings(const std::string &vertexSource,
                                                            const std::string &fragmentSource,
                                                            const std::string &name, bool makeCurrent)
{
    logInfo("Loading shader program '" + name + "' from strings", "ShaderManager");

    auto vertexShader = std::make_shared<VertexShader>();
    vertexShader->setSource(vertexSource);

    auto fragmentShader = std::make_shared<FragmentShader>();
    fragmentShader->setSource(fragmentSource);

    return buildProgram(vertexShader, fragmentShader, name, makeCurrent);
}

ProgramPtr ShaderManager::loadProgramFromFiles(const std::string vertexShader, const std::string fragmentShader,
                                               const std::string name, bool makeCurrent)
{
    return compileProgramFromFiles(vertexShader, fragmentShader, name, makeCurrent).value_or(nullptr);
}

ProgramPtr ShaderManager::loadProgramFromStrings(const std::string vertexShaderSource,
                                                 const std::string fragmentShaderSource, const std::string name,
                                                 bool makeCurrent)
{
    return compileProgramFromStrings(vertexShaderSource, fragmentShaderSource, name, makeCurrent).value_or(nullptr);
}

Result<ProgramPtr> ivf::ShaderManager::loadBasicShader()
{
    logInfo("Loading basic shader.", "ShaderManager");
    auto result = compileProgramFromStrings(ivf::basic_vert_shader_source, ivf::basic_frag_shader_source, "basic");
    if (!result)
        return result;

    auto program = *result;
    bindDefaultTexture2D(0);
    bindDefaultTexture2D(1);
    bindDefaultTexture2D(2);
    bindDefaultTexture2D(3);
    bindDefaultTexture2D(4);
    bindDefaultCubemap(kDefaultCubemapUnit);
    program->uniformInt("texture0", 0);
    program->uniformInt("envMap", kDefaultCubemapUnit);
    return program;
}

Result<ProgramPtr> ivf::ShaderManager::loadPBRShader()
{
    logInfo("Loading PBR shader.", "ShaderManager");
    auto previousProgram = m_currentProgram;
    auto result = compileProgramFromStrings(ivf::pbr_vert_shader_source, ivf::pbr_frag_shader_source, "pbr", false);
    if (!result)
        return result;

    auto program = *result;
    bindDefaultTexture2D(0);
    bindDefaultTexture2D(1);
    bindDefaultTexture2D(2);
    bindDefaultTexture2D(3);
    bindDefaultTexture2D(4);
    bindDefaultCubemap(kDefaultCubemapUnit);
    program->use();
    program->uniformInt("albedoMap", 0);
    program->uniformInt("normalMap", 1);
    program->uniformInt("roughnessMap", 2);
    program->uniformInt("metallicMap", 3);
    program->uniformInt("aoMap", 4);
    program->uniformInt("envCubemap", kDefaultCubemapUnit);

    if (previousProgram)
        previousProgram->use();

    return program;
}

Result<ProgramPtr> ivf::ShaderManager::loadBumpShader()
{
    logInfo("Loading bump shader.", "ShaderManager");
    auto previousProgram = m_currentProgram;
    auto result = compileProgramFromStrings(ivf::bump_vert_shader_source, ivf::bump_frag_shader_source, "bump", false);
    if (!result)
        return result;

    auto program = *result;
    bindDefaultTexture2D(0);
    bindDefaultTexture2D(1);
    bindDefaultCubemap(kDefaultCubemapUnit);
    program->use();
    program->uniformInt("texture0", 0);
    program->uniformInt("normalMap", 1);
    program->uniformInt("envMap", kDefaultCubemapUnit);

    if (previousProgram)
        previousProgram->use();

    return program;
}

Result<ProgramPtr> ivf::ShaderManager::loadRenderToTextureShader()
{
    logInfo("Loading render to texture shader.", "ShaderManager");
    return compileProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                     ivf::render_to_texture_frag_shader_source_330, "render_to_texture");
}

std::shared_ptr<Program> ShaderManager::currentProgram()
{
    return m_currentProgram;
}

bool ivf::ShaderManager::setCurrentProgram(const std::string name)
{
    auto it = m_programs.find(name);
    if (it != m_programs.end())
    {
        m_currentProgram = it->second;
        m_currentProgram->use();
        return true;
    }
    else
    {
        return false;
    }
}

bool ivf::ShaderManager::setCurrentProgram(ProgramPtr program)
{
    if (!program)
        return false;

    m_currentProgram = program;
    m_currentProgram->use();
    return true;
}

ProgramPtr ivf::ShaderManager::program(const std::string name)
{
    auto it = m_programs.find(name);
    if (it != m_programs.end())
        return it->second;
    else
        return nullptr;
}

void ShaderManager::apply()
{
    m_currentProgram->use();
}

void ivf::ShaderManager::applyProgram(const std::string name)
{
    this->setCurrentProgram(name);
    this->apply();
}

bool ivf::ShaderManager::compileLinkErrors()
{
    return m_linkErrors || m_vertexCompileErrors || m_fragCompileErrors;
}

ProgramPtr ivf::smLoadProgramFromFiles(const std::string vertexShader, const std::string fragmentShader,
                                       const std::string name, bool makeCurrent)
{
    return ShaderManager::instance()
        ->compileProgramFromFiles(vertexShader, fragmentShader, name, makeCurrent)
        .value_or(nullptr);
}

ProgramPtr ivf::smLoadProgramFromStrings(const std::string vertexShaderSource, const std::string fragmentShaderSource,
                                         const std::string name, bool makeCurrent)
{
    return ShaderManager::instance()
        ->compileProgramFromStrings(vertexShaderSource, fragmentShaderSource, name, makeCurrent)
        .value_or(nullptr);
}

Result<ProgramPtr> ivf::smLoadBasicShader()
{
    return ShaderManager::instance()->loadBasicShader();
}

Result<ProgramPtr> ivf::smLoadPBRShader()
{
    return ShaderManager::instance()->loadPBRShader();
}

Result<ProgramPtr> ivf::smLoadBumpShader()
{
    return ShaderManager::instance()->loadBumpShader();
}

Result<ProgramPtr> ivf::smLoadRenderToTextureShader()
{
    return ShaderManager::instance()->loadRenderToTextureShader();
}

ProgramPtr ivf::smCurrentProgram()
{
    return ShaderManager::instance()->currentProgram();
}

bool ivf::smSetCurrentProgram(const std::string name)
{
    return ShaderManager::instance()->setCurrentProgram(name);
}

void ivf::smApply()
{
    return ShaderManager::instance()->apply();
}

void ivf::smApplyProgram(const std::string name)
{
    return ShaderManager::instance()->applyProgram(name);
}

// Deprecated wrapper calling a deprecated member.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
bool ivf::smCompileLinkErrors()
{
    return ShaderManager::instance()->compileLinkErrors();
}
#if defined(_MSC_VER)
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif
