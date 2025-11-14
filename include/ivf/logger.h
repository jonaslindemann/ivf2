#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace ivf {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    None
};

class Logger {
private:
    Logger();
    static Logger *m_instance;

    LogLevel m_logLevel;
    bool m_consoleOutput;
    bool m_fileOutput;
    std::string m_logFilePath;
    std::ofstream m_logFile;
    bool m_showTimestamp;

    std::string levelToString(LogLevel level) const;
    std::string getTimestamp() const;
    void writeLog(LogLevel level, const std::string &message);

public:
    virtual ~Logger();
    static Logger *instance();
    static Logger *create();
    static void drop();

    void setLogLevel(LogLevel level);
    LogLevel logLevel() const;
    void setConsoleOutput(bool enable);
    bool consoleOutput() const;
    bool setFileOutput(const std::string &filePath);
    void disableFileOutput();
    bool fileOutput() const;
    void setShowTimestamp(bool enable);
    bool showTimestamp() const;

    void debug(const std::string &message);
    void info(const std::string &message);
    void warning(const std::string &message);
    void error(const std::string &message);
};

typedef Logger *LoggerPtr;

void logDebug(const std::string &message);
void logInfo(const std::string &message);
void logWarning(const std::string &message);
void logError(const std::string &message);
void setLogLevel(LogLevel level);
void setConsoleOutput(bool enable);
bool setFileOutput(const std::string &filePath);
void disableFileOutput();
void setShowTimestamp(bool enable);

} // namespace ivf
