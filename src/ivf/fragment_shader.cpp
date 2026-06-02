#include <ivf/fragment_shader.h>

#include <ivf/logger.h>

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
    int infoLogLength = 0;
    
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 1)
    {
        std::vector<char> fragmentShaderErrorMessage(infoLogLength);
        glGetShaderInfoLog(m_id, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);

        if (result == GL_FALSE)
            logErrorfc("FragmentShader", "Fragment shader compilation errors:");
        else
            logWarningfc("FragmentShader", "Fragment shader compilation warnings:");

        if (fragmentShaderErrorMessage[0] != '\0')
        {
            if (result == GL_FALSE)
                logErrorfc("FragmentShader", "{}", &fragmentShaderErrorMessage[0]);
            else
                logWarningfc("FragmentShader", "{}", &fragmentShaderErrorMessage[0]);
        }
    }

    if (result == GL_FALSE)
        return false;

    return true;
}
