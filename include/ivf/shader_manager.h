#pragma once

#include <ivf/fragment_shader.h>
#include <ivf/program.h>
#include <ivf/transform_manager.h>
#include <ivf/vertex_shader.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * @brief The namespace ivf contains classes and functions related to the IVF (Interactive Visualization Framework)
 * library.
 */
namespace ivf {

/**
 * @class ShaderManager
 * @brief Singleton class for managing shaders and shader programs.
 *
 * The ShaderManager class provides functionality to load, compile, and link shaders,
 * as well as manage shader programs. It allows loading shaders from files or strings,
 * and provides methods to retrieve the current shader program and apply it.
 * The class also provides error checking for shader compilation and linking.
 */
class ShaderManager {
private:
    ShaderManager();                  ///< Private constructor for singleton pattern.
    static ShaderManager *m_instance; ///< Singleton instance pointer.

    bool m_linkErrors;          ///< True if there were linking errors.
    bool m_vertexCompileErrors; ///< True if there were vertex shader compile errors.
    bool m_fragCompileErrors;   ///< True if there were fragment shader compile errors.

    std::map<std::string, std::shared_ptr<Program>> m_programs; ///< Map of program name to ProgramPtr.
    ProgramPtr m_currentProgram;                                ///< Currently active shader program.

public:
    /**
     * @brief Destructor.
     */
    virtual ~ShaderManager();

    /**
     * @brief Get the singleton instance of the ShaderManager.
     * @return ShaderManager* Pointer to the singleton instance.
     */
    static ShaderManager *instance();

    /**
     * @brief Create the singleton instance of the ShaderManager (if not already created).
     * @return ShaderManager* Pointer to the singleton instance.
     */
    static ShaderManager *create();

    /**
     * @brief Destroy the singleton instance and release all resources.
     */
    static void drop();

    /**
     * @brief Load and link a shader program from vertex and fragment shader files.
     * @param vertexShader Path to the vertex shader file.
     * @param fragmentShader Path to the fragment shader file.
     * @param name Name to assign to the program.
     * @param makeCurrent If true, set as the current program.
     * @return ProgramPtr Shared pointer to the loaded program.
     */
    ProgramPtr loadProgramFromFiles(const std::string vertexShader, const std::string fragmentShader,
                                    const std::string name, bool makeCurrent = true);

    /**
     * @brief Load and link a shader program from vertex and fragment shader source strings.
     * @param vertexShaderSource Vertex shader source code.
     * @param fragmentShaderSource Fragment shader source code.
     * @param name Name to assign to the program.
     * @param makeCurrent If true, set as the current program.
     * @return ProgramPtr Shared pointer to the loaded program.
     */
    ProgramPtr loadProgramFromStrings(const std::string vertexShaderSource, const std::string fragmentShaderSource,
                                      const std::string name, bool makeCurrent = true);

    /**
     * @brief Load a basic default shader program.
     * @return ProgramPtr Shared pointer to the basic shader program.
     */
    ProgramPtr loadBasicShader();

    /**
     * @brief Load a shader program for rendering to a texture.
     * @return ProgramPtr Shared pointer to the render-to-texture shader program.
     */
    ProgramPtr loadRenderToTextureShader();

    /**
     * @brief Get the currently active shader program.
     * @return ProgramPtr Shared pointer to the current program.
     */
    ProgramPtr currentProgram();

    /**
     * @brief Set the current shader program by name.
     * @param name Name of the program.
     * @return bool True if the program was found and set.
     */
    bool setCurrentProgram(const std::string name);

    /**
     * @brief Get a shader program by name.
     * @param name Name of the program.
     * @return ProgramPtr Shared pointer to the program.
     */
    ProgramPtr program(const std::string name);

    /**
     * @brief Apply the current shader program (bind for use).
     */
    void apply();

    /**
     * @brief Apply a specific shader program by name (bind for use).
     * @param name Name of the program.
     */
    void applyProgram(const std::string name);

    /**
     * @brief Check if there were any compile or link errors.
     * @return bool True if there were errors.
     */
    bool compileLinkErrors();
};

/**
 * @typedef ShaderManagerPtr
 * @brief Pointer type for ShaderManager singleton.
 */
typedef ShaderManager *ShaderManagerPtr;

/**
 * @brief Load and link a shader program from files using the global ShaderManager.
 */
ProgramPtr smLoadProgramFromFiles(const std::string vertexShader, const std::string fragmentShader,
                                  const std::string name, bool makeCurrent = true);

/**
 * @brief Load and link a shader program from source strings using the global ShaderManager.
 */
ProgramPtr smLoadProgramFromStrings(const std::string vertexShaderSource, const std::string fragmentShaderSource,
                                    const std::string name, bool makeCurrent = true);

/**
 * @brief Load a basic default shader program using the global ShaderManager.
 */
ProgramPtr smLoadBasicShader();

/**
 * @brief Load a render-to-texture shader program using the global ShaderManager.
 */
ProgramPtr smLoadRenderToTextureShader();

/**
 * @brief Get the currently active shader program using the global ShaderManager.
 */
ProgramPtr smCurrentProgram();

/**
 * @brief Set the current shader program by name using the global ShaderManager.
 */
bool smSetCurrentProgram(const std::string name);

/**
 * @brief Apply the current shader program using the global ShaderManager.
 */
void smApply();

/**
 * @brief Apply a specific shader program by name using the global ShaderManager.
 */
void smApplyProgram(const std::string name);

/**
 * @brief Check for compile or link errors using the global ShaderManager.
 */
bool smCompileLinkErrors();

}; // namespace ivf
