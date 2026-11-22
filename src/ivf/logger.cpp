#include <ivf/logger.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>

using namespace ivf;
using namespace std;

Logger *Logger::m_instance = nullptr;

Logger::Logger() 
    : m_logLevel(LogLevel::Info), 
      m_consoleOutput(true), 
      m_fileOutput(false), 
      m_showTimestamp(true), 
      m_showContext(true),
      m_timestampWidth(0),   // pad timestamps to at least this width (they have a fixed length anyway)
      m_levelWidth(5),       // DEBUG / INFO / WARN / ERROR
      m_contextWidth(16),    // longer contexts are shortened with '~'
      m_showDate(false),     // time only: HH:MM:SS.mmm
      m_wrapColumn(100)      // wrap long messages at column 100
{}

Logger::~Logger() { 
    if (m_logFile.is_open()) 
        m_logFile.close(); 
}

Logger *Logger::instance() { 
    if (!m_instance) 
        m_instance = new Logger(); 
    return m_instance; 
}

Logger *Logger::create() { return instance(); }

void Logger::drop() { 
    delete m_instance; 
    m_instance = nullptr; 
}

void Logger::setLogLevel(LogLevel level) { m_logLevel = level; }
LogLevel Logger::logLevel() const { return m_logLevel; }
void Logger::setConsoleOutput(bool enable) { m_consoleOutput = enable; }
bool Logger::consoleOutput() const { return m_consoleOutput; }

bool Logger::setFileOutput(const std::string &filePath) { 
    if (m_logFile.is_open()) 
        m_logFile.close(); 
    m_logFilePath = filePath; 
    m_logFile.open(m_logFilePath, std::ios::app); 
    m_fileOutput = m_logFile.is_open(); 
    return m_fileOutput; 
}

void Logger::disableFileOutput() { 
    if (m_logFile.is_open()) 
        m_logFile.close(); 
    m_fileOutput = false; 
}

bool Logger::fileOutput() const { return m_fileOutput; }
void Logger::setShowTimestamp(bool enable) { m_showTimestamp = enable; }
bool Logger::showTimestamp() const { return m_showTimestamp; }
void Logger::setShowContext(bool enable) { m_showContext = enable; }
bool Logger::showContext() const { return m_showContext; }
void Logger::setShowDate(bool enable) { m_showDate = enable; }
bool Logger::showDate() const { return m_showDate; }
void Logger::setWrapColumn(int column) { m_wrapColumn = column < 0 ? 0 : column; }
int Logger::wrapColumn() const { return m_wrapColumn; }

// Field width configuration methods
void Logger::setTimestampWidth(int width) { m_timestampWidth = width; }
int Logger::timestampWidth() const { return m_timestampWidth; }
void Logger::setLevelWidth(int width) { m_levelWidth = width; }
int Logger::levelWidth() const { return m_levelWidth; }
void Logger::setContextWidth(int width) { m_contextWidth = width; }
int Logger::contextWidth() const { return m_contextWidth; }
void Logger::setFieldWidths(int timestampWidth, int levelWidth, int contextWidth) {
    m_timestampWidth = timestampWidth;
    m_levelWidth = levelWidth;
    m_contextWidth = contextWidth;
}

std::string Logger::levelToString(LogLevel level) const { 
    switch (level) { 
        case LogLevel::Debug: return "DEBUG"; 
        case LogLevel::Info: return "INFO"; 
        case LogLevel::Warning: return "WARN"; 
        case LogLevel::Error: return "ERROR"; 
        case LogLevel::None: return "NONE"; 
        default: return "UNKNOWN"; 
    } 
}

std::string Logger::timestamp() const { 
    auto now = std::chrono::system_clock::now(); 
    auto timeT = std::chrono::system_clock::to_time_t(now); 
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; 
    std::stringstream ss; 
    ss << std::put_time(std::localtime(&timeT), m_showDate ? "%Y-%m-%d %H:%M:%S" : "%H:%M:%S"); 
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count(); 
    return ss.str(); 
}

namespace {

// Pad (or shorten with '~') text to exactly width characters.
std::string fitColumn(const std::string &text, int width)
{
    if (width <= 0)
        return text;
    const auto w = static_cast<size_t>(width);
    if (text.size() > w)
        return w > 1 ? text.substr(0, w - 1) + "~" : text.substr(0, w);
    return text + std::string(w - text.size(), ' ');
}

// Split one line into pieces of at most width characters, breaking at spaces.
std::vector<std::string> wrapLine(const std::string &line, size_t width)
{
    std::vector<std::string> pieces;
    std::string rest = line;
    while (rest.size() > width)
    {
        size_t cut = rest.rfind(' ', width);
        if (cut == std::string::npos || cut == 0)
            cut = width; // no space: hard break inside the word
        pieces.push_back(rest.substr(0, cut));
        rest = rest.substr(cut);
        const size_t first = rest.find_first_not_of(' ');
        rest = first == std::string::npos ? std::string() : rest.substr(first);
    }
    pieces.push_back(rest);
    return pieces;
}

} // namespace

std::string Logger::formatMessage(LogLevel level, const std::string &message, const std::string &context) const
{
    std::string prefix;
    if (m_showTimestamp)
    {
        const std::string stamp = timestamp();
        prefix += fitColumn(stamp, std::max(m_timestampWidth, static_cast<int>(stamp.size()))) + ' ';
    }
    prefix += fitColumn(levelToString(level), m_levelWidth) + ' ';
    if (m_showContext)
        prefix += fitColumn(context, m_contextWidth) + ' ';

    // Message lines: embedded line breaks, then optional wrapping at m_wrapColumn.
    constexpr size_t minMessageWidth = 20;
    const size_t width = m_wrapColumn > 0
                             ? std::max(minMessageWidth, static_cast<size_t>(m_wrapColumn) > prefix.size()
                                                             ? static_cast<size_t>(m_wrapColumn) - prefix.size()
                                                             : size_t{0})
                             : std::string::npos;

    std::vector<std::string> lines;
    size_t pos = 0;
    while (true)
    {
        const size_t next = message.find('\n', pos);
        std::string line = message.substr(pos, next == std::string::npos ? std::string::npos : next - pos);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (width == std::string::npos)
            lines.push_back(line);
        else
            for (auto &piece : wrapLine(line, width))
                lines.push_back(std::move(piece));
        if (next == std::string::npos)
            break;
        pos = next + 1;
    }
    while (lines.size() > 1 && lines.back().empty()) // drop trailing line breaks
        lines.pop_back();

    std::string result = prefix + lines.front();
    const std::string indent(prefix.size(), ' ');
    for (size_t i = 1; i < lines.size(); ++i)
        result += "\n" + indent + lines[i];

    // No trailing spaces when the message is empty or the context column is padded
    while (!result.empty() && result.back() == ' ')
        result.pop_back();
    return result;
}

int Logger::addSink(LogSink sink)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const int id = m_nextSinkId++;
    m_sinks.emplace_back(id, std::move(sink));
    return id;
}

void Logger::removeSink(int id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::erase_if(m_sinks, [id](const auto &entry) { return entry.first == id; });
}

void Logger::writeLog(LogLevel level, const std::string &message, const std::string &context) { 
    if (m_logLevel == LogLevel::None || level < m_logLevel) 
        return; 

    std::vector<std::pair<int, LogSink>> sinks;
    {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    const std::string finalMessage = formatMessage(level, message, context);

    if (m_consoleOutput) { 
        if (level == LogLevel::Error) 
            std::cerr << finalMessage << std::endl; 
        else 
            std::cout << finalMessage << std::endl; 
    } 
    
    if (m_fileOutput && m_logFile.is_open()) { 
        m_logFile << finalMessage << std::endl; 
        m_logFile.flush(); 
    } 

    sinks = m_sinks;
    }

    // Outside the lock, so a sink may log without deadlocking.
    for (const auto &entry : sinks)
        entry.second(level, message, context);
}

void Logger::debug(const std::string &message, const std::string &context) { writeLog(LogLevel::Debug, message, context); }
void Logger::info(const std::string &message, const std::string &context) { writeLog(LogLevel::Info, message, context); }
void Logger::warning(const std::string &message, const std::string &context) { writeLog(LogLevel::Warning, message, context); }
void Logger::error(const std::string &message, const std::string &context) { writeLog(LogLevel::Error, message, context); }

void ivf::logDebug(const std::string &message, const std::string &context) { Logger::instance()->debug(message, context); }
void ivf::logInfo(const std::string &message, const std::string &context) { Logger::instance()->info(message, context); }
void ivf::logWarning(const std::string &message, const std::string &context) { Logger::instance()->warning(message, context); }
void ivf::logError(const std::string &message, const std::string &context) { Logger::instance()->error(message, context); }
void ivf::setLogLevel(LogLevel level) { Logger::instance()->setLogLevel(level); }
void ivf::setConsoleOutput(bool enable) { Logger::instance()->setConsoleOutput(enable); }
bool ivf::setFileOutput(const std::string &filePath) { return Logger::instance()->setFileOutput(filePath); }
void ivf::disableFileOutput() { Logger::instance()->disableFileOutput(); }
void ivf::setShowTimestamp(bool enable) { Logger::instance()->setShowTimestamp(enable); }
void ivf::setShowContext(bool enable) { Logger::instance()->setShowContext(enable); }
void ivf::setShowDate(bool enable) { Logger::instance()->setShowDate(enable); }
void ivf::setWrapColumn(int column) { Logger::instance()->setWrapColumn(column); }

// Field width configuration functions
void ivf::setTimestampWidth(int width) { Logger::instance()->setTimestampWidth(width); }
void ivf::setLevelWidth(int width) { Logger::instance()->setLevelWidth(width); }
void ivf::setContextWidth(int width) { Logger::instance()->setContextWidth(width); }
void ivf::setFieldWidths(int timestampWidth, int levelWidth, int contextWidth) { 
    Logger::instance()->setFieldWidths(timestampWidth, levelWidth, contextWidth); 
}