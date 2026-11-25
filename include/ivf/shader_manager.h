#pragma once

#include <ivf/fragment_shader.h>
#include <ivf/program.h>
#include <ivf/result.h>
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
    friend class RenderContext; // owns and creates the manager

private:
    ShaderManager();                  ///< Private constructor for singleton pattern.

    bool m_linkErrors;          ///< True if there were linking errors.
    bool m_vertexCompileErrors; ///< True if there were vertex shader compile errors.
    bool m_fragCompileErrors;   ///< True if there were fragment shader compile errors.

    std::map<std::string, std::shared_ptr<Program>> m_programs; ///< Map of program name to ProgramPtr.
    ProgramPtr m_currentProgram;                                ///< Currently active shader program.
    bool m_programLocked{false};                                ///< setCurrentProgram() keeps the current program.

public:
    /**
     * @brief Destructor.
     */
    virtual ~ShaderManager();

    /**
     * @brief The ShaderManager of the current RenderContext (RenderContext::current()->shaders()).
     * @return ShaderManager* Pointer to the manager.
     */
    static ShaderManager *instance();

    /**
     * @brief Same as instance(); kept for compatibility.
     * @return ShaderManager* Pointer to the manager.
     */
    static ShaderManager *create();

    /**
     * @brief Deprecated no-op: managers are owned and released by their RenderContext.
     */
    static void drop();

    /**
     * @brief Compile and link a shader program from vertex and fragment shader files.
     *
     * On success the program is registered under @p name (replacing an earlier one).
     * On failure nothing is registered and the error says which file or stage failed;
     * for compile and link errors its message ends with the driver's output.
     * @code
     * auto program = shaderMgr->compileProgramFromFiles("shaders/wave.vert", "shaders/wave.frag", "wave");
     * if (!program)
     *     return -1; // the reason is already logged; program.error().message has the details
     * (*program)->uniformFloat("amplitude", 0.5f);
     * @endcode
     * @param vertexFile Path to the vertex shader file.
     * @param fragmentFile Path to the fragment shader file.
     * @param name Name to assign to the program.
     * @param makeCurrent If true, set as the current program.
     * @return The program, or an error (FileNotFound, ReadFailed, CompileFailed, LinkFailed).
     */
    [[nodiscard]] Result<ProgramPtr> compileProgramFromFiles(const std::string &vertexFile,
                                                             const std::string &fragmentFile,
                                                             const std::string &name, bool makeCurrent = true);

    /**
     * @brief Compile and link a shader program from vertex and fragment shader source strings.
     * @param vertexSource Vertex shader source code.
     * @param fragmentSource Fragment shader source code.
     * @param name Name to assign to the program.
     * @param makeCurrent If true, set as the current program.
     * @return The program, or an error (CompileFailed, LinkFailed).
     */
    [[nodiscard]] Result<ProgramPtr> compileProgramFromStrings(const std::string &vertexSource,
                                                               const std::string &fragmentSource,
                                                               const std::string &name, bool makeCurrent = true);

    /**
     * @brief Deprecated: use compileProgramFromFiles(), which reports why it failed.
     * @return The program, or nullptr if it could not be built (the reason is logged).
     */
    [[deprecated("Use compileProgramFromFiles(), which returns an ivf::Result")]] ProgramPtr loadProgramFromFiles(
        const std::string &vertexShader, const std::string &fragmentShader, const std::string &name,
        bool makeCurrent = true);

    /**
     * @brief Deprecated: use compileProgramFromStrings(), which reports why it failed.
     * @return The program, or nullptr if it could not be built (the reason is logged).
     */
    [[deprecated("Use compileProgramFromStrings(), which returns an ivf::Result")]] ProgramPtr loadProgramFromStrings(
        const std::string &vertexShaderSource, const std::string &fragmentShaderSource, const std::string &name,
        bool makeCurrent = true);

    /**
     * @brief Load the basic default shader program, registered as "basic", and make it current.
     * @return The program, or an error if it failed to compile or link.
     */
    Result<ProgramPtr> loadBasicShader();

    /**
     * @brief Load the PBR (physically-based rendering) shader program.
     * Registered under the name "pbr". Does not change the current program.
     * @return The program, or an error if it failed to compile or link.
     */
    Result<ProgramPtr> loadPBRShader();

    /**
     * @brief Load the bump-mapping (normal-map Phong) shader program.
     * Registered under the name "bump". Does not change the current program.
     * @return The program, or an error if it failed to compile or link.
     */
    Result<ProgramPtr> loadBumpShader();

    /**
     * @brief Load a shader program for rendering to a texture, registered as "render_to_texture".
     * @return The program, or an error if it failed to compile or link.
     */
    Result<ProgramPtr> loadRenderToTextureShader();

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
    bool setCurrentProgram(const std::string &name);

    /**
     * @brief Set the current shader program directly.
     *
     * The program does not need to be registered with the manager.
     * @param program Program to make current.
     * @return bool True if the program was non-null and set.
     */
    bool setCurrentProgram(ProgramPtr program);

    /**
     * @brief Keep the current program: setCurrentProgram() does nothing until unlockProgram().
     *
     * Used for the shadow pass, where every node is drawn with the depth program whatever
     * its material uses (materials can still set uniforms on the current program).
     */
    void lockProgram();

    /** @brief Let setCurrentProgram() change the program again. */
    void unlockProgram();

    /** @brief True between lockProgram() and unlockProgram(). */
    [[nodiscard]] bool programLocked() const;

    /**
     * @brief Get a shader program by name.
     * @param name Name of the program.
     * @return ProgramPtr Shared pointer to the program.
     */
    ProgramPtr program(const std::string &name);

    /**
     * @brief Apply the current shader program (bind for use).
     */
    void apply();

    /**
     * @brief Apply a specific shader program by name (bind for use).
     * @param name Name of the program.
     */
    void applyProgram(const std::string &name);

    /**
     * @brief Deprecated: check the Result returned when the program is built instead.
     * @return bool True if the most recently built program had compile or link errors.
     */
    [[deprecated("Check the ivf::Result returned by compileProgramFrom*() or load*Shader()")]] bool
    compileLinkErrors();

private:
    Result<ProgramPtr> buildProgram(std::shared_ptr<VertexShader> vertexShader,
                                    std::shared_ptr<FragmentShader> fragmentShader, const std::string &name,
                                    bool makeCurrent);
};

/**
 * @typedef ShaderManagerPtr
 * @brief Pointer type for ShaderManager singleton.
 */
typedef ShaderManager *ShaderManagerPtr;

/**
 * @brief Deprecated: use ShaderManager::compileProgramFromFiles().
 */
[[deprecated("Use ShaderManager::compileProgramFromFiles(), which returns an ivf::Result")]] ProgramPtr
smLoadProgramFromFiles(const std::string &vertexShader, const std::string &fragmentShader, const std::string &name,
                       bool makeCurrent = true);

/**
 * @brief Deprecated: use ShaderManager::compileProgramFromStrings().
 */
[[deprecated("Use ShaderManager::compileProgramFromStrings(), which returns an ivf::Result")]] ProgramPtr
smLoadProgramFromStrings(const std::string &vertexShaderSource, const std::string &fragmentShaderSource,
                         const std::string &name, bool makeCurrent = true);

/**
 * @brief Load a basic default shader program using the global ShaderManager.
 */
Result<ProgramPtr> smLoadBasicShader();

/**
 * @brief Load the PBR shader program using the global ShaderManager.
 */
Result<ProgramPtr> smLoadPBRShader();

/**
 * @brief Load the bump-mapping shader program using the global ShaderManager.
 */
Result<ProgramPtr> smLoadBumpShader();

/**
 * @brief Load a render-to-texture shader program using the global ShaderManager.
 */
Result<ProgramPtr> smLoadRenderToTextureShader();

/**
 * @brief Get the currently active shader program using the global ShaderManager.
 */
ProgramPtr smCurrentProgram();

/**
 * @brief Set the current shader program by name using the global ShaderManager.
 */
bool smSetCurrentProgram(const std::string &name);

/**
 * @brief Apply the current shader program using the global ShaderManager.
 */
void smApply();

/**
 * @brief Apply a specific shader program by name using the global ShaderManager.
 */
void smApplyProgram(const std::string &name);

/**
 * @brief Deprecated: check the Result returned when the program is built instead.
 */
[[deprecated("Check the ivf::Result returned by compileProgramFrom*() or load*Shader()")]] bool smCompileLinkErrors();

}; // namespace ivf
