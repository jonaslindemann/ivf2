// Hot reload without OpenGL: change detection with debouncing, failures, unwatch,
// and reloading glTF models in place.

#include "test_main.h"

#include <ivf/gltf_model.h>
#include <ivf/render_context.h>
#include <ivf/resource_watcher.h>

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

using namespace ivf;
namespace fs = std::filesystem;

namespace {

std::shared_ptr<RenderContext> makeContext()
{
    auto ctx = RenderContext::create(nullptr);
    ctx->makeCurrent();
    return ctx;
}

std::string tempFile(const std::string &name, const std::string &contents)
{
    const auto path = (fs::temp_directory_path() / name).string();
    std::ofstream(path) << contents;
    return path;
}

/// Pretend the file was saved again: a modification time later than any set before (two quick
/// writes can get the same time from the file system, so the times here only go up)
void touch(const std::string &path)
{
    static int saves = 0;
    fs::last_write_time(path, fs::file_time_type::clock::now() + std::chrono::seconds(10 + 2 * ++saves));
}

const char *gltfWithNodes(int count)
{
    return count == 1 ? R"({"asset":{"version":"2.0"},"scenes":[{"nodes":[0]}],"nodes":[{"name":"a"}]})"
                      : R"({"asset":{"version":"2.0"},"scenes":[{"nodes":[0,1]}],"nodes":[{"name":"a"},{"name":"b"}]})";
}

} // namespace

IVF_TEST(reloads_once_the_file_has_settled)
{
    auto ctx = makeContext();
    auto *watcher = ctx->resourceWatcher();
    const auto file = tempFile("ivf2_test_watch.txt", "one");

    int calls = 0;
    watcher->watch({file}, [&] { ++calls; return true; }, "text");
    IVF_CHECK(watcher->size() == 1);

    watcher->check();
    IVF_CHECK(calls == 0); // unchanged

    touch(file);
    watcher->check();
    IVF_CHECK(calls == 0); // changed: wait until it stops changing
    watcher->check();
    IVF_CHECK(calls == 1);
    IVF_CHECK(watcher->reloadCount() == 1);
    watcher->check();
    IVF_CHECK(calls == 1); // once per change

    // A file still being written (time changes between checks) waits
    touch(file);
    watcher->check();
    touch(file);
    watcher->check();
    IVF_CHECK(calls == 1);
    watcher->check();
    IVF_CHECK(calls == 2);

    RenderContext::clearCurrent();
    std::remove(file.c_str());
}

IVF_TEST(failed_reloads_wait_for_the_next_save)
{
    auto ctx = makeContext();
    auto *watcher = ctx->resourceWatcher();
    const auto file = tempFile("ivf2_test_watch_fail.txt", "x");

    int calls = 0;
    std::string lastName;
    bool lastOk = true;
    watcher->setOnReload([&](const std::string &name, bool ok) { lastName = name; lastOk = ok; });
    watcher->watch({file}, [&] { ++calls; return false; }, "broken");

    touch(file);
    watcher->check();
    watcher->check();
    IVF_CHECK(calls == 1);
    IVF_CHECK(!lastOk && lastName == "broken");
    IVF_CHECK(watcher->reloadCount() == 0);
    watcher->check();
    IVF_CHECK(calls == 1); // not retried until the file changes again

    touch(file);
    watcher->check();
    watcher->check();
    IVF_CHECK(calls == 2);

    RenderContext::clearCurrent();
    std::remove(file.c_str());
}

IVF_TEST(several_files_unwatch_update_and_enabled)
{
    auto ctx = makeContext();
    auto *watcher = ctx->resourceWatcher();
    const auto a = tempFile("ivf2_test_watch_a.txt", "a");
    const auto b = tempFile("ivf2_test_watch_b.txt", "b");

    int calls = 0;
    const auto id = watcher->watch({a, b}, [&] { ++calls; return true; });
    touch(b); // either file
    watcher->check();
    watcher->check();
    IVF_CHECK(calls == 1);

    IVF_CHECK(watcher->reloadNow(id));
    IVF_CHECK(calls == 2);

    // update() checks at the interval; disabled, it does nothing
    touch(a);
    watcher->setEnabled(false);
    watcher->update(1.0f);
    watcher->update(1.0f);
    IVF_CHECK(calls == 2);
    watcher->setEnabled(true);
    watcher->update(0.2f); // below the interval
    watcher->update(0.4f);
    watcher->update(0.6f);
    IVF_CHECK(calls == 3);

    watcher->unwatch(id);
    IVF_CHECK(watcher->size() == 0);
    touch(a);
    watcher->check();
    watcher->check();
    IVF_CHECK(calls == 3);
    IVF_CHECK(!watcher->reloadNow(id));

    // A missing file is skipped, not reloaded
    watcher->watch({"ivf2_no_such_file.txt"}, [&] { ++calls; return true; });
    watcher->check();
    watcher->check();
    IVF_CHECK(calls == 3);
    watcher->clear();
    IVF_CHECK(watcher->size() == 0);

    RenderContext::clearCurrent();
    std::remove(a.c_str());
    std::remove(b.c_str());
}

IVF_TEST(gltf_models_reload_in_place)
{
    auto ctx = makeContext();
    auto *watcher = ctx->resourceWatcher();
    const auto file = tempFile("ivf2_test_watch_model.gltf", gltfWithNodes(1));

    auto loaded = GltfModel::load(file);
    IVF_CHECK(loaded.has_value());
    if (!loaded)
        return;
    auto model = *loaded;
    model->setPos({1.0f, 2.0f, 3.0f});
    IVF_CHECK(model->nodeCount() == 1);
    IVF_CHECK(model->path() == file);

    int reloads = 0;
    watcher->watchModel(model, file, [&](CompositeNode &node) {
        ++reloads;
        IVF_CHECK(&node == model.get());
    });

    std::ofstream(file) << gltfWithNodes(2);
    touch(file);
    watcher->check();
    watcher->check();
    IVF_CHECK(reloads == 1);
    IVF_CHECK(model->nodeCount() == 2); // the same object, new contents
    IVF_CHECK(model->count() == 2);
    IVF_CHECK(model->findNode("b") != nullptr);
    IVF_CHECK(glm::length(model->pos() - glm::vec3(1.0f, 2.0f, 3.0f)) < 1e-6f);

    // A broken save keeps the model as it was
    std::ofstream(file) << "{ broken";
    touch(file);
    watcher->check();
    watcher->check();
    IVF_CHECK(reloads == 1);
    IVF_CHECK(model->nodeCount() == 2);

    // Other nodes get the loaded children (glTF through ModelLoader here)
    std::ofstream(file) << gltfWithNodes(1);
    auto group = CompositeNode::create();
    watcher->watchModel(group, file);
    touch(file);
    watcher->check();
    watcher->check();
    IVF_CHECK(group->count() == 1);
    IVF_CHECK(model->nodeCount() == 1);

    RenderContext::clearCurrent();
    std::remove(file.c_str());
}

IVF_TEST_MAIN()
