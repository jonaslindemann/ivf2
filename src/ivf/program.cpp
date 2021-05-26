#include <ivf/program.h>

using namespace ivf;

#include <iostream>
#include <algorithm>    // std::max

#include <ivf/utils.h>

using namespace std;

Program::Program()
{
    m_id = -1;
}

Program::~Program()
{
    if (m_id!=-1)
        glDeleteProgram(m_id);
}

std::shared_ptr<Program> ivf::Program::create()
{
    return std::make_shared<Program>();
}

void Program::addShader(std::shared_ptr<Shader> shader)
{
    m_shaders.push_back(shader);
}

void Program::clear()
{
    m_shaders.clear();
}

size_t Program::shaderCount()
{
    return m_shaders.size();
}

bool Program::link()
{
    if (m_id!=-1)
        glDeleteProgram(m_id);
    
    // Link the program
    fprintf(stdout, "Linking program\n");
    m_id = glCreateProgram();
    
    for (int i=0; i<(int)m_shaders.size(); i++)
        glAttachShader(m_id, m_shaders[i]->id());

    glLinkProgram(m_id);
    
    GLint result = GL_FALSE;
    int infoLogLength;
    
    // Check the program
    glGetProgramiv(m_id, GL_LINK_STATUS, &result);
    glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength!=0)
    {
        std::vector<char> programErrorMessage( std::max(infoLogLength, int(1)) );
        glGetProgramInfoLog(m_id, infoLogLength, NULL , &programErrorMessage[0]);
        fprintf(stdout, "%s\n", &programErrorMessage[0]);
        return false;
    }
    else
        return true;
}

void Program::use()
{
    glUseProgram(m_id);
}

void ivf::Program::bindAttribLoc(GLuint idx, const std::string name)
{
    glBindAttribLocation(m_id, idx, name.c_str());
}

GLint Program::attribId(const std::string name)
{
    GLint id = glGetAttribLocation(m_id, name.c_str());
    checkPrintError(__FILE__, __LINE__);
    return id;
}

GLint Program::uniformLoc(const std::string name)
{
    GLint id = glGetUniformLocation(m_id, name.c_str());
    checkPrintError(__FILE__, __LINE__);
    return id;
}

void Program::uniformMatrix4(const std::string name, glm::mat4 matrix)
{
    glUniformMatrix4fv(uniformLoc(name), 1, GL_FALSE, glm::value_ptr(matrix));
	checkPrintError(__FILE__, __LINE__);
}

void Program::uniformBool(const std::string name, bool flag)
{
	glUniform1i(uniformLoc(name), flag);
    checkPrintError(__FILE__, __LINE__);
}

void ivf::Program::uniformVec4f(const std::string name, float v0, float v1, float v2, float v3)
{
    glUniform4f(uniformLoc(name), v0, v1, v2, v3);
    checkPrintError();
}

void ivf::Program::uniformVec3f(const std::string name, float v0, float v1, float v2)
{
    glUniform3f(uniformLoc(name), v0, v1, v2);
    checkPrintError();
}

void ivf::Program::uniformVec3(const std::string name, const glm::vec3 v)
{
    glUniform3f(uniformLoc(name), v.x, v.y, v.z);
    checkPrintError();
}

void ivf::Program::uniformVec4(const std::string name, const glm::vec4 v)
{
    glUniform4f(uniformLoc(name), v.x, v.y, v.z, v.w);
    checkPrintError();
}

void ivf::Program::printAttribs()
{
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

    GLint maxLength, nAttribs;
    glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &nAttribs);
    glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

    GLchar* name = (GLchar*)malloc(maxLength);

    GLint written, size, location;
    GLenum type;

    printf(" Index | Name\n");
    printf("------------------------------------------------\n");
    for (int i = 0; i < nAttribs; i++) {
        glGetActiveAttrib(m_id, i, maxLength, &written, &size, &type, name);
        location = glGetAttribLocation(m_id, name);
        printf(" %-5d | %s\n", location, name);
    }

    free(name);
}


GLuint Program::id()
{
    return m_id;
}