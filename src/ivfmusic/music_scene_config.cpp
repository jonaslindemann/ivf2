#include <ivfmusic/music_scene_config.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

using namespace ivfmusic;

namespace {

bool parseSize(const std::string &value, int &width, int &height)
{
    return std::sscanf(value.c_str(), "%dx%d", &width, &height) == 2 && width > 0 && height > 0;
}

} // namespace

MusicSceneConfig::ParseResult MusicSceneConfig::parseArgs(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        const bool hasValue = i + 1 < argc;

        if (arg == "--help" || arg == "-h")
        {
            printUsage(argv[0]);
            return ParseResult::Exit;
        }
        else if (arg == "--fullscreen")
        {
            fullscreen = true;
        }
        else if (arg == "--present" || arg == "--show")
        {
            presentationMode = true;
            fullscreen = true;
        }
        else if (arg == "--paused")
        {
            autoplay = false;
        }
        else if (arg == "--mute")
        {
            volume = 0.0f;
        }
        else if (arg == "--size" && hasValue)
        {
            if (!parseSize(argv[++i], width, height))
            {
                std::fprintf(stderr, "Invalid size '%s', expected WxH\n", argv[i]);
                return ParseResult::Error;
            }
        }
        else if (arg == "--start" && hasValue)
        {
            startTime = std::atof(argv[++i]);
        }
        else if (arg == "--bpm" && hasValue)
        {
            bpm = std::atof(argv[++i]);
        }
        else if (arg == "--end" && hasValue)
        {
            endTime = std::atof(argv[++i]);
        }
        else if (arg == "--controls" && hasValue)
        {
            controlsFile = argv[++i];
        }
        else if (arg == "--take" && hasValue)
        {
            takeFile = argv[++i];
        }
        else if (arg == "--no-live")
        {
            liveControls = false;
        }
        else if (arg == "--export" && hasValue)
        {
            exportDir = argv[++i];
        }
        else if (arg == "--fps" && hasValue)
        {
            exportFps = std::atoi(argv[++i]);
            if (exportFps <= 0)
            {
                std::fprintf(stderr, "Invalid frame rate '%s'\n", argv[i]);
                return ParseResult::Error;
            }
        }
        else
        {
            std::fprintf(stderr, "Unknown argument '%s'\n", arg.c_str());
            printUsage(argv[0]);
            return ParseResult::Error;
        }
    }
    return ParseResult::Run;
}

std::string MusicSceneConfig::resolve(const std::string &path) const
{
    const std::string prefix = "assets/";
    if (assetSourceDir.empty() || path.compare(0, prefix.size(), prefix) != 0)
        return path;

    const std::filesystem::path source = std::filesystem::path(assetSourceDir) / path.substr(prefix.size());
    std::error_code ec;
    return std::filesystem::exists(source, ec) ? source.generic_string() : path;
}

void MusicSceneConfig::printUsage(const char *program)
{
    std::printf("Usage: %s [options]\n"
                "  --fullscreen     Run fullscreen\n"
                "  --present        Presentation mode: fullscreen, no development UI (alias --show)\n"
                "  --size WxH       Window size, e.g. 1920x1080\n"
                "  --start SECONDS  Start at this song time\n"
                "  --end SECONDS    Stop exporting at this song time (default: end of song)\n"
                "  --export DIR     Render a PNG sequence into DIR at a fixed frame rate, then exit\n"
                "  --fps FPS        Frame rate of the export (default 60)\n"
                "  --paused         Start paused\n"
                "  --mute           Start muted\n"
                "  --bpm BPM        Override the tempo\n"
                "  --controls FILE  MIDI bindings (.json) for live control\n"
                "  --take FILE      File recorded takes are saved to (default: <automation>.take.json)\n"
                "  --no-live        Ignore MIDI controllers\n"
                "  --help           Show this help\n",
                program);
}
