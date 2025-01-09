#include <ivf/vertex_shader.h>

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
    int infoLogLength;

    glGetShaderiv(m_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength != 0)
    {
        std::cout << "Vertex shader compilation errors:" << std::endl;
        std::vector<char> vertexShaderErrorMessage(infoLogLength);
        glGetShaderInfoLog(m_id, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &vertexShaderErrorMessage[0]);
        return false;
    }
    else
        return true;
}
