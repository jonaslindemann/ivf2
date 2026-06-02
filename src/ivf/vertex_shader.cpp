#include <ivf/vertex_shader.h>

#include <ivf/logger.h>

using namespace ivf;

#include <iostream>
#include <vector>

using namespace std;

VertexShader::VertexShader(const std::string filename) : Shader(filename)
{}

VertexShader::~VertexShader()
{
    if (m_id != -1)
        glDeleteShader(m_id);
}

std::shared_ptr<VertexShader> ivf::VertexShader::create(const std::string filename)
{
    return std::make_shared<VertexShader>(filename);
}

bool VertexShader::compile()
{
    // Delete existing shader if any

    if (m_id != -1)
        glDeleteShader(m_id);

    // Create new shader from source

    m_id = glCreateShader(GL_VERTEX_SHADER);
    const char *source = m_shaderCode.c_str();
    glShaderSource(m_id, 1, &source, 0);
    glCompileShader(m_id);

    // Check compilation status

    GLint result = GL_FALSE;
    int infoLogLength = 0;

    glGetShaderiv(m_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 1)
    {
        std::vector<char> vertexShaderErrorMessage(infoLogLength);
        glGetShaderInfoLog(m_id, infoLogLength, NULL, &vertexShaderErrorMessage[0]);

        if (result == GL_FALSE)
            logErrorfc("VertexShader", "Vertex shader compilation errors:");
        else
            logWarningfc("VertexShader", "Vertex shader compilation warnings:");

        if (vertexShaderErrorMessage[0] != '\0')
        {
            if (result == GL_FALSE)
                logErrorfc("VertexShader", "{}", &vertexShaderErrorMessage[0]);
            else
                logWarningfc("VertexShader", "{}", &vertexShaderErrorMessage[0]);
        }
    }

    if (result == GL_FALSE)
        return false;

    return true;
}
