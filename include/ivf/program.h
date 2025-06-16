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

/**
 * @class Program
 * @brief Manages an OpenGL shader program and its associated shaders and uniforms.
 *
 * The Program class encapsulates the creation, linking, and management of an OpenGL shader
 * program. It supports attaching multiple shaders, binding attribute locations, querying
 * uniform/attribute locations, and setting uniform values for various types. The class
 * provides a high-level interface for working with GLSL programs in modern OpenGL.
 */
class Program : public GLBase {
protected:
    std::string m_name;                             ///< Name of the program (for identification).
    std::vector<std::shared_ptr<Shader>> m_shaders; ///< Attached shaders.
    GLuint m_id;                                    ///< OpenGL program object ID.
    bool m_enabled;                                 ///< Whether the program is currently enabled.

public:
    /**
     * @brief Default constructor.
     */
    Program();

    /**
     * @brief Destructor.
     */
    virtual ~Program();

    /**
     * @brief Factory method to create a shared pointer to a Program instance.
     * @return std::shared_ptr<Program> New Program instance.
     */
    static std::shared_ptr<Program> create();

    /**
     * @brief Attach a shader to the program.
     * @param shader Shared pointer to the shader.
     */
    void addShader(std::shared_ptr<Shader> shader);

    /**
     * @brief Remove all attached shaders from the program.
     */
    void clear();

    /**
     * @brief Get the number of attached shaders.
     * @return size_t Number of shaders.
     */
    size_t shaderCount();

    /**
     * @brief Link the attached shaders into a complete program.
     * @return bool True if linking succeeded.
     */
    bool link();

    /**
     * @brief Activate the program for use in OpenGL.
     */
    void use();

    /**
     * @brief Check if the program is currently enabled.
     * @return bool True if the program is enabled.
     */
    bool enabled() const;

    /**
     * @brief Set the program as enabled or disabled.
     * @param enabled True to enable, false to disable.
     */
    void setEnabled(bool enabled);

    /**
     * @brief Bind an attribute location to a named variable.
     * @param idx Attribute index.
     * @param name Attribute variable name.
     */
    void bindAttribLoc(GLuint idx, const std::string name);

    /**
     * @brief Get the attribute location for a named variable.
     * @param name Attribute variable name.
     * @return GLint Attribute location.
     */
    GLint attribId(const std::string name);

    /**
     * @brief Get the uniform location for a named variable.
     * @param name Uniform variable name.
     * @return GLint Uniform location.
     */
    GLint uniformLoc(const std::string name);

    /**
     * @brief Set a mat4 uniform by name.
     * @param name Uniform variable name.
     * @param matrix Matrix value.
     */
    void uniformMatrix4(const std::string name, glm::mat4 matrix);

    /**
     * @brief Set a mat4 uniform by location.
     * @param id Uniform location.
     * @param matrix Matrix value.
     */
    void uniformMatrix4(GLint id, glm::mat4 matrix);

    /**
     * @brief Set a bool uniform by name.
     * @param name Uniform variable name.
     * @param flag Boolean value.
     */
    void uniformBool(const std::string name, bool flag);

    /**
     * @brief Set a bool uniform by location.
     * @param id Uniform location.
     * @param flag Boolean value.
     */
    void uniformBool(GLint id, bool flag);

    /**
     * @brief Set an int uniform by name.
     * @param name Uniform variable name.
     * @param value Integer value.
     */
    void uniformInt(const std::string name, int value);

    /**
     * @brief Set an int uniform by location.
     * @param id Uniform location.
     * @param value Integer value.
     */
    void uniformInt(GLint id, int value);

    /**
     * @brief Set a uint uniform by name.
     * @param name Uniform variable name.
     * @param value Unsigned integer value.
     */
    void uniformUInt(const std::string name, int value);

    /**
     * @brief Set a uint uniform by location.
     * @param id Uniform location.
     * @param value Unsigned integer value.
     */
    void uniformUInt(GLint id, int value);

    /**
     * @brief Set a float uniform by name.
     * @param name Uniform variable name.
     * @param value Float value.
     */
    void uniformFloat(const std::string name, float value);

    /**
     * @brief Set a float uniform by location.
     * @param id Uniform location.
     * @param value Float value.
     */
    void uniformFloat(GLint id, float value);

    /**
     * @brief Set a vec4 uniform by name.
     * @param name Uniform variable name.
     * @param v0 First component.
     * @param v1 Second component.
     * @param v2 Third component.
     * @param v3 Fourth component.
     */
    void uniformVec4f(const std::string name, float v0, float v1, float v2, float v3);

    /**
     * @brief Set a vec4 uniform by location.
     * @param id Uniform location.
     * @param v0 First component.
     * @param v1 Second component.
     * @param v2 Third component.
     * @param v3 Fourth component.
     */
    void uniformVec4f(GLint id, float v0, float v1, float v2, float v3);

    /**
     * @brief Set a vec3 uniform by name.
     * @param name Uniform variable name.
     * @param v0 First component.
     * @param v1 Second component.
     * @param v2 Third component.
     */
    void uniformVec3f(const std::string name, float v0, float v1, float v2);

    /**
     * @brief Set a vec3 uniform by location.
     * @param id Uniform location.
     * @param v0 First component.
     * @param v1 Second component.
     * @param v2 Third component.
     */
    void uniformVec3f(GLint id, float v0, float v1, float v2);

    /**
     * @brief Set a vec3 uniform by name using a glm::vec3.
     * @param name Uniform variable name.
     * @param v Vector value.
     */
    void uniformVec3(const std::string name, const glm::vec3 v);

    /**
     * @brief Set a vec3 uniform by location using a glm::vec3.
     * @param id Uniform location.
     * @param v Vector value.
     */
    void uniformVec3(GLint id, const glm::vec3 v);

    /**
     * @brief Set a vec4 uniform by name using a glm::vec4.
     * @param name Uniform variable name.
     * @param v Vector value.
     */
    void uniformVec4(const std::string name, const glm::vec4 v);

    /**
     * @brief Set a vec4 uniform by location using a glm::vec4.
     * @param id Uniform location.
     * @param v Vector value.
     */
    void uniformVec4(GLint id, const glm::vec4 v);

    /**
     * @brief Set a mat4 uniform by name using a glm::mat4.
     * @param name Uniform variable name.
     * @param matrix Matrix value.
     */
    void uniformMat4(const std::string name, const glm::mat4 matrix);

    /**
     * @brief Set a mat4 uniform by location using a glm::mat4.
     * @param id Uniform location.
     * @param matrix Matrix value.
     */
    void uniformMat4(GLint id, const glm::mat4 matrix);

    /**
     * @brief Set an int array uniform by name.
     * @param name Uniform variable name.
     * @param count Number of elements.
     * @param values Pointer to the int array.
     */
    void uniformIntArray(const std::string name, int count, const int *values);

    /**
     * @brief Set a mat4 array uniform by name.
     * @param name Uniform variable name.
     * @param count Number of matrices.
     * @param matrices Pointer to the array of glm::mat4.
     */
    void uniformMatrix4Array(const std::string name, int count, const glm::mat4 *matrices);

    /**
     * @brief Print all active attribute locations and names.
     */
    void printAttribs();

    /**
     * @brief Get the OpenGL program object ID.
     * @return GLuint Program ID.
     */
    GLuint id();

    /**
     * @brief Get the name of the program.
     * @return std::string Program name.
     */
    std::string name();

    /**
     * @brief Set the name of the program.
     * @param name Program name.
     */
    void setName(std::string name);

protected:
    /**
     * @brief Hook for derived classes to set custom parameters after linking.
     */
    virtual void doParams();
};

/**
 * @typedef ProgramPtr
 * @brief Shared pointer type for Program.
 */
typedef std::shared_ptr<Program> ProgramPtr;

}; // namespace ivf
