// ivf::Result error handling: makeError() and loaders that fail before touching OpenGL.

#include "test_main.h"

#include <ivf/config.h>
#include <ivf/logger.h>
#include <ivf/model_loader.h>
#include <ivf/result.h>
#include <ivf/scene_serializer.h>

#include <string>
#include <vector>

using namespace ivf;

namespace {

// Collect logged errors instead of printing them.
struct CapturedErrors {
    std::vector<std::string> messages;
    int sink;
    CapturedErrors()
    {
        Logger::instance()->setConsoleOutput(false);
        sink = Logger::instance()->addSink([this](LogLevel level, const std::string &msg, const std::string &) {
            if (level == LogLevel::Error)
                messages.push_back(msg);
        });
    }
    ~CapturedErrors() { Logger::instance()->removeSink(sink); }
};

Result<int> parsePositive(int value)
{
    if (value <= 0)
        return makeError(ErrorCode::InvalidArgument, "value must be positive", "Test");
    return value;
}

} // namespace

IVF_TEST(make_error_logs_and_returns_error)
{
    CapturedErrors log;
    auto bad = parsePositive(-1);
    IVF_CHECK(!bad);
    IVF_CHECK(!bad && bad.error().code == ErrorCode::InvalidArgument);
    IVF_CHECK(!bad && bad.error().message == "value must be positive");
    IVF_CHECK(log.messages.size() == 1 && log.messages[0] == "value must be positive");

    auto good = parsePositive(3);
    IVF_CHECK(good && *good == 3);
    IVF_CHECK(parsePositive(-5).value_or(42) == 42);
}

IVF_TEST(silent_error_does_not_log)
{
    CapturedErrors log;
    Result<int> r = silentError(ErrorCode::Unknown, "quiet");
    IVF_CHECK(!r && r.error().message == "quiet");
    IVF_CHECK(log.messages.empty());
}

IVF_TEST(error_code_names)
{
    IVF_CHECK(std::string(toString(ErrorCode::FileNotFound)) == "file not found");
    IVF_CHECK(std::string(toString(ErrorCode::NotSupported)) == "not supported");
}

IVF_TEST(model_loader_reports_missing_file)
{
    CapturedErrors log;
    auto model = ModelLoader::load("does/not/exist.obj");
    IVF_CHECK(!model);
#if IVF2_HAS_ASSIMP
    IVF_CHECK(!model && model.error().code == ErrorCode::FileNotFound);
#else
    // Built without assimp (e.g. the "minimal" preset): no model can be loaded at all
    IVF_CHECK(!model && model.error().code == ErrorCode::NotSupported);
#endif
    IVF_CHECK(!model && model.error().message.find("does/not/exist.obj") != std::string::npos);
    IVF_CHECK(log.messages.size() == 1);
}

IVF_TEST(scene_serializer_reports_missing_file)
{
    CapturedErrors log;
    auto scene = SceneSerializer::loadFile("does/not/exist.json");
    IVF_CHECK(!scene && scene.error().code == ErrorCode::FileNotFound);

    // The deprecated wrapper keeps its old behaviour: nullptr on failure.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    auto legacy = SceneSerializer::load("does/not/exist.json");
#if defined(_MSC_VER)
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif
    IVF_CHECK(legacy == nullptr);
}

IVF_TEST_MAIN()
