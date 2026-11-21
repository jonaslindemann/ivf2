// Logger sinks: level filtering, removal, re-entrancy and concurrent logging.

#include "test_main.h"

#include <ivf/logger.h>

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using ivf::Logger;
using ivf::LogLevel;

namespace {

struct Quiet {
    Quiet()
    {
        Logger::instance()->setConsoleOutput(false);
        Logger::instance()->setLogLevel(LogLevel::Info);
    }
};
Quiet quiet;

} // namespace

IVF_TEST(sink_receives_messages_above_level)
{
    std::vector<std::string> got;
    const int id = Logger::instance()->addSink(
        [&](LogLevel, const std::string &msg, const std::string &ctx) { got.push_back(ctx + ":" + msg); });

    ivf::logDebug("hidden", "T"); // below Info
    ivf::logInfo("hello", "T");
    ivf::logError("bad", "T");
    Logger::instance()->removeSink(id);

    IVF_CHECK(got.size() == 2);
    IVF_CHECK(got.size() == 2 && got[0] == "T:hello" && got[1] == "T:bad");
}

IVF_TEST(removed_sink_receives_nothing)
{
    int calls = 0;
    const int id = Logger::instance()->addSink([&](LogLevel, const std::string &, const std::string &) { ++calls; });
    Logger::instance()->removeSink(id);
    ivf::logInfo("after removal", "T");
    IVF_CHECK(calls == 0);
}

IVF_TEST(sink_may_log_without_deadlock)
{
    int calls = 0;
    const int id = Logger::instance()->addSink([&](LogLevel, const std::string &msg, const std::string &) {
        ++calls;
        if (msg == "outer")
            ivf::logInfo("inner", "T"); // re-entrant logging from inside a sink
    });
    ivf::logInfo("outer", "T");
    Logger::instance()->removeSink(id);
    IVF_CHECK(calls == 2);
}

IVF_TEST(concurrent_logging_delivers_every_message)
{
    std::atomic<int> count{0};
    const int id = Logger::instance()->addSink([&](LogLevel, const std::string &, const std::string &) { ++count; });

    constexpr int threads = 4, perThread = 500;
    std::vector<std::thread> pool;
    for (int t = 0; t < threads; ++t)
        pool.emplace_back([t] {
            for (int i = 0; i < perThread; ++i)
                ivf::logInfo(ivf::logStr("thread ", t, " message ", i), "T");
        });
    for (auto &th : pool)
        th.join();

    Logger::instance()->removeSink(id);
    IVF_CHECK(count == threads * perThread);
}

// Format without timestamps so the output is deterministic.
struct FormatSettings {
    FormatSettings(int contextWidth = 16, int wrap = 0)
    {
        auto l = Logger::instance();
        l->setShowTimestamp(false);
        l->setContextWidth(contextWidth);
        l->setWrapColumn(wrap);
    }
    ~FormatSettings()
    {
        auto l = Logger::instance();
        l->setShowTimestamp(true);
        l->setContextWidth(16);
        l->setWrapColumn(100); // the default
    }
};

IVF_TEST(format_aligned_columns)
{
    FormatSettings fs;
    auto l = Logger::instance();
    IVF_CHECK(l->formatMessage(LogLevel::Info, "Hello", "App") == "INFO  App              Hello");
    IVF_CHECK(l->formatMessage(LogLevel::Warning, "Hello", "App") == "WARN  App              Hello");
    IVF_CHECK(l->formatMessage(LogLevel::Error, "Hello", "App") == "ERROR App              Hello");
}

IVF_TEST(format_long_context_is_shortened)
{
    FormatSettings fs;
    IVF_CHECK(Logger::instance()->formatMessage(LogLevel::Info, "x", "GLFWMusicSceneWindow") ==
              "INFO  GLFWMusicSceneW~ x");
}

IVF_TEST(format_wraps_under_message_column)
{
    FormatSettings fs(16, 50); // prefix is 23 characters, so 27 for the message
    const std::string out =
        Logger::instance()->formatMessage(LogLevel::Info, "one two three four five six seven eight nine", "App");
    const std::string indent(23, ' ');
    // "one two three four five six" is exactly 27 characters, so it fits on the first line
    IVF_CHECK(out == "INFO  App              one two three four five six\n" + indent + "seven eight nine");
}

IVF_TEST(format_indents_embedded_line_breaks)
{
    FormatSettings fs;
    const std::string out = Logger::instance()->formatMessage(LogLevel::Error, "line 1\nline 2\n", "Shader");
    IVF_CHECK(out == "ERROR Shader           line 1\n" + std::string(23, ' ') + "line 2");
}

IVF_TEST(format_timestamp_time_only_by_default)
{
    auto l = Logger::instance();
    const std::string out = l->formatMessage(LogLevel::Info, "m", "C");
    // HH:MM:SS.mmm followed by a space, then the level
    IVF_CHECK(out.size() > 13 && out[2] == ':' && out[5] == ':' && out[8] == '.' && out[12] == ' ');
    l->setShowDate(true);
    const std::string dated = l->formatMessage(LogLevel::Info, "m", "C");
    l->setShowDate(false);
    IVF_CHECK(dated.size() > 24 && dated[4] == '-' && dated[7] == '-' && dated[10] == ' ');
}

IVF_TEST(logStr_streams_like_cout)
{
    IVF_CHECK(ivf::logStr("a", 1, ' ', 2.5, "b") == "a1 2.5b");
}

IVF_TEST_MAIN()
