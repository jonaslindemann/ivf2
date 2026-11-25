#pragma once

/**
 * @file resource_watcher.h
 * @brief Hot reload: reload textures, models, shaders or anything else when their files change.
 *
 * @code
 * auto texture = ivf::Texture::create();
 * if (!texture->load("assets/brick.png"))
 *     return -1;
 * auto *watcher = renderContext()->resourceWatcher();
 * watcher->watchTexture(texture);              // reloaded into the same OpenGL texture
 *
 * auto model = ivf::GltfModel::load("assets/models/robot.glb");
 * if (!model)
 *     return -1;
 * add(*model);
 * watcher->watchModel(*model, "assets/models/robot.glb");
 *
 * // Anything else: files and a function that reloads them (return false to keep the old state)
 * watcher->watch({"data/level.json"}, [this] { return loadLevel(); }, "level");
 * @endcode
 *
 * GLFWSceneWindow calls update() every frame. Files are checked twice a second; a file is
 * reloaded once its modification time has stayed the same for one check, so editors that
 * save in several steps do not trigger reloads of half-written files. If a reload fails
 * (a broken file, a shader that does not compile), the old resource is kept and the error
 * is logged once; the next save tries again. ShaderWatcher uses this watcher.
 */

#include <ivf/composite_node.h>
#include <ivf/texture.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace ivf {

class ResourceWatcher {
    friend class RenderContext; // owns and creates the watcher

public:
    using WatchId = std::uint64_t;
    using ReloadFunction = std::function<bool()>;

    /** @brief The watcher of the current RenderContext. */
    static ResourceWatcher *instance();

    /**
     * @brief Watch @p files and call @p reload when one of them changes.
     * @param reload Reloads the resource; returns false if that failed (the old state is kept).
     * @param name Name used in log messages (default: the first file).
     * @return Id for unwatch() and reloadNow().
     */
    WatchId watch(std::vector<std::string> files, ReloadFunction reload, std::string name = {});

    /**
     * @brief Reload @p texture from @p path when the file changes (into the same OpenGL
     *        texture, so materials using it see the new image).
     * @param path Image file; empty: the file the texture was loaded from (Texture::filename()).
     */
    WatchId watchTexture(TexturePtr texture, std::string path = {});

    /**
     * @brief Reload a model when its file changes, replacing the node's contents in place.
     *
     * A GltfModel reloads itself (GltfModel::reload(), keeping its animation and time); any
     * other node gets the children of ModelLoader::load(@p path). @p onReload is called after
     * a successful reload, for example to set materials again.
     */
    WatchId watchModel(CompositeNodePtr node, std::string path,
                       std::function<void(CompositeNode &)> onReload = {});

    /** @brief Stop watching. */
    void unwatch(WatchId id);

    /** @brief Stop watching everything. */
    void clear();

    /** @brief Number of watched resources. */
    [[nodiscard]] std::size_t size() const;

    /**
     * @brief Check the files every @p checkInterval seconds (call every frame with the
     *        frame time; GLFWSceneWindow does).
     */
    void update(float dt, float checkInterval = 0.5f);

    /** @brief Check the files now (a changed file is reloaded on the next check that finds it unchanged). */
    void check();

    /** @brief Reload a resource now, changed or not. @return Whether the reload succeeded. */
    bool reloadNow(WatchId id);

    /** @brief Switch watching on or off (default on). */
    void setEnabled(bool flag);
    [[nodiscard]] bool enabled() const;

    /** @brief Number of successful reloads so far. */
    [[nodiscard]] int reloadCount() const;

    /** @brief Called after every reload attempt with the resource's name and the result (e.g. for a status line). */
    void setOnReload(std::function<void(const std::string &name, bool ok)> callback);

private:
    ResourceWatcher() = default;

    struct Entry {
        WatchId id{0};
        std::string name;
        std::vector<std::string> files;
        std::vector<std::filesystem::file_time_type> times;   ///< Times of the last reload (or watch()).
        std::vector<std::filesystem::file_time_type> pending; ///< Changed times waiting to settle.
        bool hasPending{false};
        ReloadFunction reload;
    };

    std::vector<Entry> m_entries;
    WatchId m_nextId{1};
    float m_accum{0.0f};
    bool m_enabled{true};
    int m_reloadCount{0};
    std::function<void(const std::string &, bool)> m_onReload;

    bool runReload(Entry &entry);
};

} // namespace ivf
