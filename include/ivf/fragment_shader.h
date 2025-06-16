#pragma once

#include <ivf/shader.h>

#include <string>

namespace ivf {

/**
 * @class FragmentShader
 * @brief Represents an OpenGL fragment shader.
 *
 * The FragmentShader class encapsulates the creation, compilation, and management of
 * an OpenGL fragment shader. It inherits from Shader and provides methods for loading
 * shader code from a file and compiling it for use in a rendering pipeline.
 */
class FragmentShader : public Shader {
private:
    // No additional private members.
public:
    /**
     * @brief Constructor.
     * @param filename Path to the fragment shader source file (optional).
     */
    FragmentShader(const std::string filename = "");

    /**
     * @brief Destructor.
     */
    virtual ~FragmentShader();

    /**
     * @brief Factory method to create a shared pointer to a FragmentShader instance.
     * @param filename Path to the fragment shader source file (optional).
     * @return std::shared_ptr<FragmentShader> New FragmentShader instance.
     */
    static std::shared_ptr<FragmentShader> create(const std::string filename = "");

    /**
     * @brief Compile the fragment shader from source.
     * @return true if compilation was successful, false otherwise.
     */
    virtual bool compile();
};

/**
 * @typedef FragmentShaderPtr
 * @brief Shared pointer type for FragmentShader.
 */
typedef std::shared_ptr<FragmentShader> FragmentShaderPtr;

}; // namespace ivf
