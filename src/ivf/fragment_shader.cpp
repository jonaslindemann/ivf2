#include <ivf/fragment_shader.h>

using namespace ivf;

#include <iostream>
#include <vector>

using namespace std;

FragmentShader::FragmentShader(const std::string filename)
:Shader(filename)
{
}

FragmentShader::~FragmentShader()
{
    if (m_id!=-1)
        glDeleteShader(m_id);
}

std::shared_ptr<FragmentShader> ivf::FragmentShader::create(const std::string filename)
{
    return std::make_shared<FragmentShader>(filename);
}

bool FragmentShader::compile()
{
    // Delete existing shader if any
    
    if (m_id!=-1)
        glDeleteShader(m_id);
    
    // Create new shader from source
    
    m_id = glCreateShader(GL_FRAGMENT_SHADER);
    const char* source = m_shaderCode.c_str();
    glShaderSource(m_id, 1, &source, 0);
    glCompileShader(m_id);
    
    // Check compilation status
    
    GLint result = GL_FALSE;
    int infoLogLength;
    
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength!=0)
    {
        std::vector<char> fragmentShaderErrorMessage(infoLogLength);
        glGetShaderInfoLog(m_id, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &fragmentShaderErrorMessage[0]);
        return false;
    }
    else
        return true;
}
