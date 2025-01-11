#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ivf {

class Program : public GLBase {
protected:
    std::string m_name;
    std::vector<std::shared_ptr<Shader>> m_shaders;
    GLuint m_id;

public:
    Program();
    virtual ~Program();

    static std::shared_ptr<Program> create();

    void addShader(std::shared_ptr<Shader> shader);
    void clear();
    size_t shaderCount();

    bool link();
    void use();

    void bindAttribLoc(GLuint idx, const std::string name);

    GLint attribId(const std::string name);
    GLint uniformLoc(const std::string name);

    void uniformMatrix4(const std::string name, glm::mat4 matrix);
    void uniformMatrix4(GLint id, glm::mat4 matrix);

    void uniformBool(const std::string name, bool flag);
    void uniformBool(GLint id, bool flag);

    void uniformInt(const std::string name, int value);
    void uniformInt(GLint id, int value);

    void uniformUInt(const std::string name, int value);
    void uniformUInt(GLint id, int value);

    void uniformFloat(const std::string name, float value);
    void uniformFloat(GLint id, float value);

    void uniformVec4f(const std::string name, float v0, float v1, float v2, float v3);
    void uniformVec4f(GLint id, float v0, float v1, float v2, float v3);
    void uniformVec3f(const std::string name, float v0, float v1, float v2);
    void uniformVec3f(GLint id, float v0, float v1, float v2);
    void uniformVec3(const std::string name, const glm::vec3 v);
    void uniformVec3(GLint id, const glm::vec3 v);
    void uniformVec4(const std::string name, const glm::vec4 v);
    void uniformVec4(GLint id, const glm::vec4 v);

    void printAttribs();

    GLuint id();
    std::string name();
    void setName(std::string name);

protected:
    virtual void doParams();
};

typedef std::shared_ptr<Program> ProgramPtr;

}; // namespace ivf
