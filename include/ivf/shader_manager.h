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
 * @brief A class that manages shaders and shader programs.
 *
 * The ShaderManager class provides functionality to load, compile, and link shaders,
 * as well as manage shader programs. It allows loading shaders from files or strings,
 * and provides methods to retrieve the current shader program and apply it.
 * The class also provides error checking for shader compilation and linking.
 */
class ShaderManager {
private:
    ShaderManager();
    static ShaderManager *m_instance;

    bool m_linkErrors;
    bool m_vertexCompileErrors;
    bool m_fragCompileErrors;

    std::map<std::string, std::shared_ptr<Program>> m_programs;
    ProgramPtr m_currentProgram;

public:
    virtual ~ShaderManager();

    static ShaderManager *instance();
    static ShaderManager *create();
    static void drop();

    ProgramPtr loadProgramFromFiles(const std::string vertexShader, const std::string fragmentShader,
                                    const std::string name);

    ProgramPtr loadProgramFromStrings(const std::string vertexShaderSource, const std::string fragmentShaderSource,
                                      const std::string name);

    ProgramPtr loadBasicShader();
    ProgramPtr currentProgram();

    bool setCurrentProgram(const std::string name);

    void apply();

    bool compileLinkErrors();
};

typedef ShaderManager *ShaderManagerPtr;

}; // namespace ivf
