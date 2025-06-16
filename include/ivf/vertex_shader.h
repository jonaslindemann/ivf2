#pragma once

/**
 * @file vertex_shader.h
 * @brief Declares the VertexShader class for managing OpenGL vertex shaders in the ivf library.
 */

#include <ivf/shader.h>
#include <string>

namespace ivf {

/**
 * @class VertexShader
 * @brief Encapsulates an OpenGL vertex shader object.
 *
 * The VertexShader class manages the loading, compilation, and resource management of
 * OpenGL vertex shaders. It supports loading shader source from a file, compiling the shader,
 * and provides a factory method for convenient creation.
 * Inherits from Shader.
 */
class VertexShader : public Shader {
private:
public:
    /**
     * @brief Construct a VertexShader, optionally loading source from a file.
     * @param filename Path to the vertex shader source file (default "").
     */
    VertexShader(const std::string filename = "");

    /**
     * @brief Destructor. Cleans up the OpenGL vertex shader object.
     */
    virtual ~VertexShader();

    /**
     * @brief Factory method to create a shared pointer to a VertexShader instance.
     * @param filename Path to the vertex shader source file (default "").
     * @return std::shared_ptr<VertexShader> New VertexShader instance.
     */
    static std::shared_ptr<VertexShader> create(const std::string filename = "");

    /**
     * @brief Compile the vertex shader from its source code.
     * @return bool True if compilation succeeded, false otherwise.
     */
    virtual bool compile();
};

/**
 * @typedef VertexShaderPtr
 * @brief Shared pointer type for VertexShader.
 */
typedef std::shared_ptr<VertexShader> VertexShaderPtr;

}; // namespace ivf
