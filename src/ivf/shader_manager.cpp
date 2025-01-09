#include <ivf/shader_manager.h>

#include <ivf/fragment_shader.h>
#include <ivf/program.h>
#include <ivf/stock_shaders.h>
#include <ivf/vertex_shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <iostream>

using namespace ivf;
using namespace std;

ShaderManager *ShaderManager::m_instance = nullptr;

ShaderManager::ShaderManager()
    : m_currentProgram(nullptr), m_linkErrors(false), m_vertexCompileErrors(false), m_fragCompileErrors(false)
{
    cout << "ShaderManager: Created." << endl;
}

ShaderManager::~ShaderManager()
{
    cout << "ShaderManager: Destroyed." << endl;
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
                                                             const std::string fragmentSource, const std::string name)
{
    cout << "ShaderManager: Loading shaders." << endl;
    cout << "\tVertex shader = " << vertexSource << endl;
    cout << "\tFragment shader = " << fragmentSource << endl;

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

    m_currentProgram = program;

    program->printAttribs();

    return program;
}

std::shared_ptr<Program> ShaderManager::loadProgramFromStrings(const std::string vertexShaderSource,
                                                               const std::string fragmentShaderSource,
                                                               const std::string name)
{
    cout << "ShaderManager: Loading shader " << name << " from string." << endl;

    auto vertexShader = std::make_shared<VertexShader>();
    vertexShader->setSource(vertexShaderSource);
    m_vertexCompileErrors = !vertexShader->compile();

    auto fragmentShader = std::make_shared<FragmentShader>();
    fragmentShader->setSource(fragmentShaderSource);
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

    m_currentProgram = program;

    program->printAttribs();

    return program;
}

std::shared_ptr<Program> ivf::ShaderManager::loadBasicShader()
{
    cout << "ShaderManager: Loading basic shader." << endl;
    return loadProgramFromStrings(ivf::basic_vert_shader_source, ivf::basic_frag_shader_source, "basic");
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
        cout << "ShaderManager: Setting current program to " << name << endl;
        m_currentProgram = it->second;
        m_currentProgram->use();
        return true;
    }
    else
    {
        cout << "ShaderManager: Program " << name << " not found." << endl;
        return false;
    }
}

void ShaderManager::apply()
{
    m_currentProgram->use();
}

bool ivf::ShaderManager::compileLinkErrors()
{
    return m_linkErrors || m_vertexCompileErrors || m_fragCompileErrors;
}
