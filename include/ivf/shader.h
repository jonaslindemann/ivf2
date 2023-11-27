#pragma once

#include <ivf/glbase.h>

#include <string>

namespace ivf
{

class Shader : public GLBase
{
  protected:
    std::string m_filename;
    std::string m_shaderCode;
    GLuint m_id;

  public:
    Shader(const std::string filename = "");
    virtual ~Shader();

    static std::shared_ptr<Shader> create(const std::string filename = "");

    void setFilename(const std::string filename);
    const std::string filename();

    void setSource(const std::string source);

    const std::string source();

    bool read();

    virtual bool compile();

    GLuint id();
};

typedef std::shared_ptr<Shader> ShaderPtr;

}; // namespace ivf
