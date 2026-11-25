#include <ivf/resource_watcher.h>

#include <ivf/gltf_model.h>
#include <ivf/logger.h>
#include <ivf/model_loader.h>
#include <ivf/render_context.h>

#include <algorithm>

using namespace ivf;

namespace {

bool readTimes(const std::vector<std::string> &files, std::vector<std::filesystem::file_time_type> &times)
{
    times.resize(files.size());
    for (std::size_t i = 0; i < files.size(); ++i)
    {
        std::error_code ec;
        times[i] = std::filesystem::last_write_time(files[i], ec);
        if (ec)
            return false; // missing (e.g. while an editor replaces the file): try again later
    }
    return true;
}

} // namespace

// The watcher lives in the current RenderContext (see ivf/render_context.h).
ResourceWatcher *ResourceWatcher::instance()
{
    return RenderContext::currentOrFallback()->resourceWatcher();
}

ResourceWatcher::WatchId ResourceWatcher::watch(std::vector<std::string> files, ReloadFunction reload, std::string name)
{
    Entry entry;
    entry.id = m_nextId++;
    entry.name = !name.empty() ? std::move(name) : (files.empty() ? std::string("resource") : files.front());
    entry.files = std::move(files);
    entry.reload = std::move(reload);
    readTimes(entry.files, entry.times);
    m_entries.push_back(std::move(entry));
    return m_entries.back().id;
}

ResourceWatcher::WatchId ResourceWatcher::watchTexture(TexturePtr texture, std::string path)
{
    if (path.empty() && texture)
        path = texture->filename();
    if (!texture || path.empty())
    {
        logWarning("watchTexture(): no texture or file to watch", "ResourceWatcher");
        return 0;
    }
    return watch({path}, [texture, path] { return texture->load(path).has_value(); }, path);
}

ResourceWatcher::WatchId ResourceWatcher::watchModel(CompositeNodePtr node, std::string path,
                                                     std::function<void(CompositeNode &)> onReload)
{
    if (!node || path.empty())
    {
        logWarning("watchModel(): no node or file to watch", "ResourceWatcher");
        return 0;
    }
    std::weak_ptr<CompositeNode> weak = node;
    auto reload = [weak, path, onReload]() {
        auto target = weak.lock();
        if (!target)
            return false;
        if (auto *model = dynamic_cast<GltfModel *>(target.get()))
        {
            if (!model->reload(path))
                return false;
        }
        else
        {
            auto loaded = ModelLoader::load(path);
            if (!loaded)
                return false;
            target->clear();
            for (auto &child : (*loaded)->nodes())
                target->add(child);
        }
        if (onReload)
            onReload(*target);
        return true;
    };
    return watch({path}, std::move(reload), path);
}

void ResourceWatcher::unwatch(WatchId id)
{
    m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(), [id](const Entry &e) { return e.id == id; }),
                    m_entries.end());
}

void ResourceWatcher::clear()
{
    m_entries.clear();
}

std::size_t ResourceWatcher::size() const
{
    return m_entries.size();
}

void ResourceWatcher::update(float dt, float checkInterval)
{
    if (!m_enabled)
        return;
    m_accum += dt;
    if (m_accum < checkInterval)
        return;
    m_accum = 0.0f;
    check();
}

void ResourceWatcher::check()
{
    std::vector<std::filesystem::file_time_type> current;
    // Reloads may add or remove entries: walk by id
    std::vector<WatchId> ids;
    ids.reserve(m_entries.size());
    for (const auto &e : m_entries)
        ids.push_back(e.id);

    for (WatchId id : ids)
    {
        auto it = std::find_if(m_entries.begin(), m_entries.end(), [id](const Entry &e) { return e.id == id; });
        if (it == m_entries.end())
            continue;
        if (!readTimes(it->files, current))
            continue;
        if (current == it->times)
        {
            it->hasPending = false;
            continue;
        }
        // Changed: reload once the file has stopped changing (same time at two checks)
        if (it->hasPending && current == it->pending)
        {
            it->hasPending = false;
            it->times = current; // also after a failure: the next save tries again
            runReload(*it);
        }
        else
        {
            it->pending = current;
            it->hasPending = true;
        }
    }
}

bool ResourceWatcher::runReload(Entry &entry)
{
    const std::string name = entry.name;
    const bool ok = entry.reload ? entry.reload() : false;
    if (ok)
    {
        ++m_reloadCount;
        logInfo("Reloaded " + name, "ResourceWatcher");
    }
    else
        logWarning("Reloading " + name + " failed; keeping the previous version", "ResourceWatcher");
    if (m_onReload)
        m_onReload(name, ok);
    return ok;
}

bool ResourceWatcher::reloadNow(WatchId id)
{
    auto it = std::find_if(m_entries.begin(), m_entries.end(), [id](const Entry &e) { return e.id == id; });
    if (it == m_entries.end())
        return false;
    readTimes(it->files, it->times);
    it->hasPending = false;
    return runReload(*it);
}

void ResourceWatcher::setEnabled(bool flag)
{
    m_enabled = flag;
}

bool ResourceWatcher::enabled() const
{
    return m_enabled;
}

int ResourceWatcher::reloadCount() const
{
    return m_reloadCount;
}

void ResourceWatcher::setOnReload(std::function<void(const std::string &, bool)> callback)
{
    m_onReload = std::move(callback);
}
