#include <ivf/program.h>

using namespace ivf;

#include <iostream>
#include <algorithm> // std::max

#include <ivf/utils.h>

using namespace std;

Program::Program() : m_id(-1), m_name("generic")
{}

Program::~Program()
{
    if (m_id != -1)
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
    if (m_id != -1)
        glDeleteProgram(m_id);

    // Link the program
    fprintf(stdout, "Linking program\n");
    m_id = glCreateProgram();

    for (int i = 0; i < (int)m_shaders.size(); i++)
        glAttachShader(m_id, m_shaders[i]->id());

    glLinkProgram(m_id);

    GLint result = GL_FALSE;
    int infoLogLength;

    // Check the program
    glGetProgramiv(m_id, GL_LINK_STATUS, &result);
    glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength != 0)
    {
        std::vector<char> programErrorMessage(std::max(infoLogLength, int(1)));
        glGetProgramInfoLog(m_id, infoLogLength, NULL, &programErrorMessage[0]);
        fprintf(stdout, "%s\n", &programErrorMessage[0]);
        return false;
    }
    else
        return true;
}

void Program::use()
{
    glUseProgram(m_id);
    this->doParams();
}

void ivf::Program::bindAttribLoc(GLuint idx, const std::string name)
{
    GL_ERR(glBindAttribLocation(m_id, idx, name.c_str()));
}

GLint Program::attribId(const std::string name)
{
    GL_ERR(GLint id = glGetAttribLocation(m_id, name.c_str()));
    return id;
}

GLint Program::uniformLoc(const std::string name)
{
#ifdef _DEBUG
    std::cout << "Getting uniform location for " << name << " in " << this->name() << std::endl;
#endif

    GL_ERR(GLint id = glGetUniformLocation(m_id, name.c_str()));
    return id;
}

void Program::uniformMatrix4(const std::string name, glm::mat4 matrix)
{
    GL_ERR(glUniformMatrix4fv(uniformLoc(name), 1, GL_FALSE, glm::value_ptr(matrix)));
}

void ivf::Program::uniformMatrix4(GLint id, glm::mat4 matrix)
{
    GL_ERR(glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(matrix)));
}

void Program::uniformBool(const std::string name, bool flag)
{
    GL_ERR(glUniform1i(uniformLoc(name), flag));
}

void ivf::Program::uniformBool(GLint id, bool flag)
{
    GL_ERR(glUniform1i(id, flag));
}

void Program::uniformInt(const std::string name, int value)
{
    GL_ERR(glUniform1i(uniformLoc(name), value));
}

void ivf::Program::uniformInt(GLint id, int value)
{
    GL_ERR(glUniform1i(id, value));
}

void ivf::Program::uniformUInt(const std::string name, int value)
{
    GL_ERR(glUniform1ui(uniformLoc(name), value));
}

void ivf::Program::uniformUInt(GLint id, int value)
{
    GL_ERR(glUniform1ui(id, value));
}

void ivf::Program::uniformFloat(const std::string name, float value)
{
    GL_ERR(glUniform1f(uniformLoc(name), value));
}

void ivf::Program::uniformFloat(GLint id, float value)
{
    GL_ERR(glUniform1f(id, value));
}

void ivf::Program::uniformVec4f(const std::string name, float v0, float v1, float v2, float v3)
{
    GL_ERR(glUniform4f(uniformLoc(name), v0, v1, v2, v3));
}

void ivf::Program::uniformVec4f(GLint id, float v0, float v1, float v2, float v3)
{
    GL_ERR(glUniform4f(id, v0, v1, v2, v3));
}

void ivf::Program::uniformVec3f(const std::string name, float v0, float v1, float v2)
{
    GL_ERR(glUniform3f(uniformLoc(name), v0, v1, v2));
}

void ivf::Program::uniformVec3f(GLint id, float v0, float v1, float v2)
{
    GL_ERR(glUniform3f(id, v0, v1, v2));
}

void ivf::Program::uniformVec3(const std::string name, const glm::vec3 v)
{
    GL_ERR(glUniform3f(uniformLoc(name), v.x, v.y, v.z));
}

void ivf::Program::uniformVec3(GLint id, const glm::vec3 v)
{
    GL_ERR(glUniform3f(id, v.x, v.y, v.z));
}

void ivf::Program::uniformVec4(const std::string name, const glm::vec4 v)
{
    GL_ERR(glUniform4f(uniformLoc(name), v.x, v.y, v.z, v.w));
}

void ivf::Program::uniformVec4(GLint id, const glm::vec4 v)
{
    GL_ERR(glUniform4f(id, v.x, v.y, v.z, v.w));
}

void ivf::Program::printAttribs()
{
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

    GLint maxLength, nAttribs;
    glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &nAttribs);
    glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

    GLchar *name = (GLchar *)malloc(maxLength);

    GLint written, size, location;
    GLenum type;

    printf(" Index | Name\n");
    printf("------------------------------------------------\n");
    for (int i = 0; i < nAttribs; i++)
    {
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

std::string ivf::Program::name()
{
    return m_name;
}

void ivf::Program::setName(std::string name)
{
    m_name = name;
}

void ivf::Program::doParams()
{}
