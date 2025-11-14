#include <ivf/shader_manager.h>

#include <ivf/fragment_shader.h>
#include <ivf/program.h>
#include <ivf/stock_shaders.h>
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
    return loadProgramFromStrings(ivf::basic_vert_shader_source, ivf::basic_frag_shader_source, "basic");
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
