#include <ivf/shadermanager.h>

#include <iostream>

using namespace ivf;
using namespace std;

ShaderManager* ShaderManager::m_instance = 0;

ShaderManager::ShaderManager()
{
    cout << "ShaderManager: Created." << endl;
    m_currentProgram = 0;
    m_linkErrors = false;
    m_vertexCompileErrors = false;
    m_fragCompileErrors = false;
}

std::shared_ptr<Program> ShaderManager::loadProgramFromFiles(const std::string vertexSource, const std::string fragmentSource)
{
    cout << "ShaderManager: Loading shaders." << endl;
    cout << "\tVertex shader = " << vertexSource << endl;
    cout << "\tFragment shader = " << fragmentSource << endl;

    std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>(vertexSource);
    m_vertexCompileErrors = !vertexShader->compile();
    
    std::shared_ptr<FragmentShader> fragmentShader = std::make_shared<FragmentShader>(fragmentSource);
    m_fragCompileErrors = !fragmentShader->compile();
    
    std::shared_ptr<Program> program = std::make_shared<Program>();
    program->addShader(vertexShader);
    program->addShader(fragmentShader);

    m_linkErrors = !program->link();

    program->bindAttribLoc(0, "aPos");
    program->bindAttribLoc(1, "aColor");
    program->bindAttribLoc(2, "aTex");
    program->bindAttribLoc(3, "aNormal");

    program->use();
    
    m_programs.push_back(program);
    m_currentProgram = program;

    program->printAttribs();
    
    return program;
}

std::shared_ptr<Program> ShaderManager::currentProgram()
{
    return m_currentProgram;
}

void ShaderManager::apply()
{
    m_currentProgram->use();
}

bool ivf::ShaderManager::compileLinkErrors()
{
    return m_linkErrors || m_vertexCompileErrors || m_fragCompileErrors;
}


