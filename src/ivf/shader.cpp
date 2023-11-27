#include <ivf/shader.h>

using namespace ivf;

#include <fstream>
#include <iostream>

using namespace std;

Shader::Shader(const std::string filename) : m_filename(filename), m_id(-1)
{
    if (filename != "")
        this->read();
}

Shader::~Shader()
{
}

std::shared_ptr<Shader> ivf::Shader::create(const std::string filename)
{
    return std::make_shared<Shader>(filename);
}

void Shader::setFilename(const std::string filename)
{
    m_filename = filename;
}

const std::string Shader::filename()
{
    return m_filename;
}

void ivf::Shader::setSource(const std::string source)
{
    m_shaderCode = source;
}

const std::string ivf::Shader::source()
{
    return m_shaderCode;
}

bool Shader::read()
{
    ifstream in(m_filename.c_str(), ios::in);

    if (in.is_open())
    {
        std::string line = "";
        m_shaderCode = "";
        while (getline(in, line))
            m_shaderCode += line + "\n";
        in.close();
        return true;
    }
    else
    {
        std::cout << "Shader: Couldn't read " << m_filename.c_str() << std::endl;
        return false;
    }
}

bool Shader::compile()
{
    return false;
}

GLuint Shader::id()
{
    return m_id;
}
