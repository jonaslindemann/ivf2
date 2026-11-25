#pragma once

/**
 * @file shader_watcher.h
 * @brief Hot-reload support for file-based GLSL shader programs.
 *
 * @code
 * auto prog = ShaderManager::instance()->compileProgramFromFiles("vs.glsl", "fs.glsl", "myShader");
 * if (prog)
 *     ShaderWatcher::instance()->watch(*prog, "vs.glsl", "fs.glsl");
 *
 * // In update loop (GLFWSceneWindow calls this automatically):
 * ShaderWatcher::instance()->update(dt);
 * @endcode
 *
 * On a file change, the watcher recompiles both shaders in place. If compilation
 * or linking fails the old program is kept and the error is logged.
 * If it succeeds, the existing ProgramPtr's GL program is replaced transparently.
 *
 * The files are watched by the context's ResourceWatcher (ivf/resource_watcher.h), which
 * GLFWSceneWindow updates every frame; update() only needs calling in other hosts.
 */

#include <ivf/program.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ivf {

class ShaderWatcher {
    friend class RenderContext; // owns and creates the manager

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
     * @brief Poll watched files and recompile any that changed (forwards to
     *        ResourceWatcher::update(), which also reloads watched textures and models).
     * @param dt            Frame delta-time in seconds.
     * @param checkInterval Minimum seconds between file checks (default: 0.5s).
     */
    void update(float dt, float checkInterval = 0.5f);

private:
    ShaderWatcher() = default;

    struct Entry {
        ProgramPtr program;
        std::string vertPath;
        std::string fragPath;
        std::uint64_t watchId{0}; ///< ResourceWatcher id.
    };

    std::vector<Entry> m_entries;

    static bool tryReload(const Entry& e);
};

} // namespace ivf
