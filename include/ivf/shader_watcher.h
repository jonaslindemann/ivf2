#pragma once

/**
 * @file shader_watcher.h
 * @brief Hot-reload support for file-based GLSL shader programs.
 *
 * @code
 * auto prog = ShaderManager::instance()->loadProgramFromFiles("vs.glsl", "fs.glsl", "myShader");
 * ShaderWatcher::instance()->watch(prog, "vs.glsl", "fs.glsl");
 *
 * // In update loop (GLFWSceneWindow calls this automatically):
 * ShaderWatcher::instance()->update(dt);
 * @endcode
 *
 * On a file change, the watcher recompiles both shaders in place. If compilation
 * or linking fails the old program is kept and the error is printed to stderr.
 * If it succeeds, the existing ProgramPtr's GL program is replaced transparently.
 *
 * GLFWSceneWindow calls ShaderWatcher::instance()->update(dt) in doUpdateOtherUi(),
 * so registered programs are automatically kept up to date.
 */

#include <ivf/program.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ivf {

class ShaderWatcher {
public:
    static ShaderWatcher* instance();

    /**
     * @brief Register a program for hot-reload.
     * @param program  The program to keep updated.
     * @param vertPath Path to the vertex shader source file.
     * @param fragPath Path to the fragment shader source file.
     */
    void watch(ProgramPtr program, std::string vertPath, std::string fragPath);

    /**
     * @brief Unwatch a specific program.
     */
    void unwatch(ProgramPtr program);

    /** Remove all watched programs. */
    void clear();

    /**
     * @brief Poll watched files and recompile any that changed.
     * @param dt            Frame delta-time in seconds.
     * @param checkInterval Minimum seconds between file checks (default: 0.5s).
     */
    void update(float dt, float checkInterval = 0.5f);

private:
    ShaderWatcher() = default;
    static ShaderWatcher* m_instance;

    struct Entry {
        ProgramPtr program;
        std::string vertPath;
        std::string fragPath;
        std::filesystem::file_time_type lastVertTime;
        std::filesystem::file_time_type lastFragTime;
    };

    std::vector<Entry> m_entries;
    float m_accum{0.0f};

    bool tryReload(Entry& e);
};

} // namespace ivf
