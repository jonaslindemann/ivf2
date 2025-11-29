#pragma once

#include <ivf/glbase.h>

#include <string>
#include <string_view>

namespace ivf {

/**
 * @class Shader
 * @brief Encapsulates an OpenGL shader object, including source code and compilation.
 *
 * The Shader class manages the loading, source code, and compilation of an OpenGL shader.
 * It supports reading shader code from a file, setting source code directly, and compiling
 * the shader for use in a shader program. The class provides access to the shader's OpenGL ID
 * and filename for resource management and debugging.
 */
class Shader : public GLBase {
protected:
    std::string m_filename;   ///< Filename of the shader source (if loaded from file).
    std::string m_shaderCode; ///< Shader source code.
    GLuint m_id;              ///< OpenGL shader object ID.

public:
    /**
     * @brief Constructor.
     * @param filename Optional filename to load shader source from.
     */
    Shader(const std::string filename = "");

    /**
     * @brief Destructor.
     */
    virtual ~Shader();

    /**
     * @brief Factory method to create a shared pointer to a Shader instance.
     * @param filename Optional filename to load shader source from.
     * @return std::shared_ptr<Shader> New Shader instance.
     */
    static std::shared_ptr<Shader> create(const std::string filename = "");

    /**
     * @brief Set the filename for the shader source.
     * @param filename Shader source filename.
     */
    void setFilename(std::string_view filename);

    /**
     * @brief Get the filename of the shader source.
     * @return const std::string Shader source filename.
     */
    const std::string filename();

    /**
     * @brief Set the shader source code directly.
     * @param source Shader source code.
     */
    void setSource(std::string_view source);

    /**
     * @brief Get the shader source code.
     * @return const std::string Shader source code.
     */
    const std::string source();

    /**
     * @brief Read the shader source code from the file.
     * @return bool True if successful.
     */
    bool read();

    /**
     * @brief Compile the shader from its source code.
     * @return bool True if compilation succeeded.
     */
    virtual bool compile();

    /**
     * @brief Get the OpenGL shader object ID.
     * @return GLuint Shader object ID.
     */
    GLuint id();
};

/**
 * @typedef ShaderPtr
 * @brief Shared pointer type for Shader.
 */
using ShaderPtr = std::shared_ptr<Shader>;

}; // namespace ivf
