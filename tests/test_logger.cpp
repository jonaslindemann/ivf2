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

IVF_TEST(logStr_streams_like_cout)
{
    IVF_CHECK(ivf::logStr("a", 1, ' ', 2.5, "b") == "a1 2.5b");
}

IVF_TEST_MAIN()
