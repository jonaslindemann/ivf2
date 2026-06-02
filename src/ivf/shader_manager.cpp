#include <ivf/shader_manager.h>

#include <ivf/fragment_shader.h>
#include <ivf/program.h>
#include <ivf/stock_shaders.h>
#include <ivf/pbr_shaders.h>
#include <ivf/bump_shaders.h>
#include <ivf/post_shaders.h>
#include <ivf/vertex_shader.h>
#include <ivf/logger.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace ivf;
using namespace std;

ShaderManager *ShaderManager::m_instance = nullptr;

namespace {

constexpr GLuint kDefaultCubemapUnit = 15;

void bindDefaultTexture2D(GLuint unit)
{
    static GLuint defaultTexture = 0;

    GLint activeTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);

    if (defaultTexture == 0)
    {
        const unsigned char pixel[] = {255, 255, 255, 255};

        glGenTextures(1, &defaultTexture);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, defaultTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, defaultTexture);
    }

    glActiveTexture(activeTexture);
}

void bindDefaultCubemap(GLuint unit)
{
    static GLuint defaultCubemap = 0;

    GLint activeTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);

    if (defaultCubemap == 0)
    {
        const unsigned char pixel[] = {0, 0, 0, 255};

        glGenTextures(1, &defaultCubemap);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, defaultCubemap);

        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; ++face)
            glTexImage2D(face, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, defaultCubemap);
    }

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

ShaderManager *ShaderManager::instance()
{
    if (!m_instance)
    {
        m_instance = new ShaderManager();
    }
    return m_instance;
}

ShaderManager *ShaderManager::create()
{
    return instance();
}

void ShaderManager::drop()
{
    delete m_instance;
    m_instance = nullptr;
}

std::shared_ptr<Program> ShaderManager::loadProgramFromFiles(const std::string vertexSource,
                                                             const std::string fragmentSource, const std::string name,
                                                             bool makeCurrent)
{
    logInfo("Loading shaders.", "ShaderManager");
    logInfo("\tVertex shader = " + vertexSource, "ShaderManager");
    logInfo("\tFragment shader = " + fragmentSource, "ShaderManager");

    auto vertexShader = std::make_shared<VertexShader>(vertexSource);
    m_vertexCompileErrors = !vertexShader->compile();

    auto fragmentShader = std::make_shared<FragmentShader>(fragmentSource);
    m_fragCompileErrors = !fragmentShader->compile();

    auto program = std::make_shared<Program>();
    program->addShader(vertexShader);
    program->addShader(fragmentShader);

    m_linkErrors = !program->link();

    program->bindAttribLoc(0, "aPos");
    program->bindAttribLoc(1, "aColor");
    program->bindAttribLoc(2, "aTex");
    program->bindAttribLoc(3, "aNormal");

    program->use();

    auto it = m_programs.find(name);

    if (it == m_programs.end())
        m_programs[name] = program;
    else
        it->second = program;

    if (makeCurrent)
        m_currentProgram = program;

    program->printAttribs();

    return program;
}

std::shared_ptr<Program> ShaderManager::loadProgramFromStrings(const std::string vertexShaderSource,
                                                               const std::string fragmentShaderSource,
                                                               const std::string name, bool makeCurrent)
{
    logInfo("Loading shader " + name + " from string.", "ShaderManager");

    auto vertexShader = std::make_shared<VertexShader>();
    vertexShader->setSource(vertexShaderSource);
    m_vertexCompileErrors = !vertexShader->compile();

    auto fragmentShader = std::make_shared<FragmentShader>();
    fragmentShader->setSource(fragmentShaderSource);
    m_fragCompileErrors = !fragmentShader->compile();

    auto program = std::make_shared<Program>();
    program->setName(name);
    program->addShader(vertexShader);
    program->addShader(fragmentShader);

    m_linkErrors = !program->link();

    program->bindAttribLoc(0, "aPos");
    program->bindAttribLoc(1, "aColor");
    program->bindAttribLoc(2, "aTex");
    program->bindAttribLoc(3, "aNormal");

    if (makeCurrent)
        program->use();

    auto it = m_programs.find(name);

    if (it == m_programs.end())
        m_programs[name] = program;
    else
        it->second = program;

    if (makeCurrent)
        m_currentProgram = program;

    program->printAttribs();

    return program;
}

std::shared_ptr<Program> ivf::ShaderManager::loadBasicShader()
{
    logInfo("Loading basic shader.", "ShaderManager");
    auto program = loadProgramFromStrings(ivf::basic_vert_shader_source, ivf::basic_frag_shader_source, "basic");
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

std::shared_ptr<Program> ivf::ShaderManager::loadPBRShader()
{
    logInfo("Loading PBR shader.", "ShaderManager");
    auto previousProgram = m_currentProgram;
    auto program = loadProgramFromStrings(ivf::pbr_vert_shader_source, ivf::pbr_frag_shader_source, "pbr", false);

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

std::shared_ptr<Program> ivf::ShaderManager::loadBumpShader()
{
    logInfo("Loading bump shader.", "ShaderManager");
    auto previousProgram = m_currentProgram;
    auto program = loadProgramFromStrings(ivf::bump_vert_shader_source, ivf::bump_frag_shader_source, "bump", false);

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

ProgramPtr ivf::ShaderManager::loadRenderToTextureShader()
{
    logInfo("Loading render to texture shader.", "ShaderManager");
    return loadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
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
    return ShaderManager::instance()->loadProgramFromFiles(vertexShader, fragmentShader, name, makeCurrent);
}

ProgramPtr ivf::smLoadProgramFromStrings(const std::string vertexShaderSource, const std::string fragmentShaderSource,
                                         const std::string name, bool makeCurrent)
{
    return ShaderManager::instance()->loadProgramFromStrings(vertexShaderSource, fragmentShaderSource, name,
                                                             makeCurrent);
}

ProgramPtr ivf::smLoadBasicShader()
{
    return ShaderManager::instance()->loadBasicShader();
}

ProgramPtr ivf::smLoadPBRShader()
{
    return ShaderManager::instance()->loadPBRShader();
}

ProgramPtr ivf::smLoadBumpShader()
{
    return ShaderManager::instance()->loadBumpShader();
}

ProgramPtr ivf::smLoadRenderToTextureShader()
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

bool ivf::smCompileLinkErrors()
{
    return ShaderManager::instance()->compileLinkErrors();
}
