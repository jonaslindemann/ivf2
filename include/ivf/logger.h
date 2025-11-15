#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <format>

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
    bool m_showContext;
    
    // Field width configuration
    int m_timestampWidth;
    int m_levelWidth;
    int m_contextWidth;

    std::string levelToString(LogLevel level) const;
    std::string getTimestamp() const;
    void writeLog(LogLevel level, const std::string &message, const std::string &context = "");

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
    void setShowContext(bool enable);
    bool showContext() const;
    
    // Field width configuration methods
    void setTimestampWidth(int width);
    int timestampWidth() const;
    void setLevelWidth(int width);
    int levelWidth() const;
    void setContextWidth(int width);
    int contextWidth() const;
    void setFieldWidths(int timestampWidth, int levelWidth, int contextWidth);

    void debug(const std::string &message, const std::string &context = "");
    void info(const std::string &message, const std::string &context = "");
    void warning(const std::string &message, const std::string &context = "");
    void error(const std::string &message, const std::string &context = "");
    
    // Formatted logging methods (without context)
    template<typename... Args>
    void debugf(std::format_string<Args...> fmt, Args&&... args) {
        writeLog(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...), "");
    }
    
    template<typename... Args>
    void infof(std::format_string<Args...> fmt, Args&&... args) {
        writeLog(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...), "");
    }
    
    template<typename... Args>
    void warningf(std::format_string<Args...> fmt, Args&&... args) {
        writeLog(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...), "");
    }
    
    template<typename... Args>
    void errorf(std::format_string<Args...> fmt, Args&&... args) {
        writeLog(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...), "");
    }
    
    // Formatted logging methods with context (context FIRST - recommended API)
    template<typename... Args>
    void debugfc(const std::string& context, std::format_string<Args...> fmt, Args&&... args) {
        writeLog(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...), context);
    }
    
    template<typename... Args>
    void infofc(const std::string& context, std::format_string<Args...> fmt, Args&&... args) {
        writeLog(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...), context);
    }
    
    template<typename... Args>
    void warningfc(const std::string& context, std::format_string<Args...> fmt, Args&&... args) {
        writeLog(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...), context);
    }
    
    template<typename... Args>
    void errorfc(const std::string& context, std::format_string<Args...> fmt, Args&&... args) {
        writeLog(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...), context);
    }
};

typedef Logger *LoggerPtr;

void logDebug(const std::string &message, const std::string &context = "");
void logInfo(const std::string &message, const std::string &context = "");
void logWarning(const std::string &message, const std::string &context = "");
void logError(const std::string &message, const std::string &context = "");

// Formatted global logging functions (without context)
template<typename... Args>
void logDebugf(std::format_string<Args...> fmt, Args&&... args) {
    Logger::instance()->debugf(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void logInfof(std::format_string<Args...> fmt, Args&&... args) {
    Logger::instance()->infof(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void logWarningf(std::format_string<Args...> fmt, Args&&... args) {
    Logger::instance()->warningf(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void logErrorf(std::format_string<Args...> fmt, Args&&... args) {
    Logger::instance()->errorf(fmt, std::forward<Args>(args)...);
}

// Formatted global logging functions with context (context FIRST - recommended API)
template<typename... Args>
void logDebugfc(const std::string& context, std::format_string<Args...> fmt, Args&&... args) {
    Logger::instance()->debugfc(context, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void logInfofc(const std::string& context, std::format_string<Args...> fmt, Args&&... args) {
    Logger::instance()->infofc(context, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void logWarningfc(const std::string& context, std::format_string<Args...> fmt, Args&&... args) {
    Logger::instance()->warningfc(context, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void logErrorfc(const std::string& context, std::format_string<Args...> fmt, Args&&... args) {
    Logger::instance()->errorfc(context, fmt, std::forward<Args>(args)...);
}

void setLogLevel(LogLevel level);
void setConsoleOutput(bool enable);
bool setFileOutput(const std::string &filePath);
void disableFileOutput();
void setShowTimestamp(bool enable);
void setShowContext(bool enable);

// Field width configuration functions
void setTimestampWidth(int width);
void setLevelWidth(int width);
void setContextWidth(int width);
void setFieldWidths(int timestampWidth, int levelWidth, int contextWidth);

} // namespace ivf

// Helper macros for automatic context capture
#define IVF_LOG_CONTEXT_FUNC std::string(__func__)
#define IVF_LOG_CONTEXT_FILE (std::string(__FILE__) + ":" + std::to_string(__LINE__))

// Macros for logging with automatic source location
#define IVF_LOG_DEBUG(msg) ivf::logDebug(msg, IVF_LOG_CONTEXT_FUNC)
#define IVF_LOG_INFO(msg) ivf::logInfo(msg, IVF_LOG_CONTEXT_FUNC)
#define IVF_LOG_WARNING(msg) ivf::logWarning(msg, IVF_LOG_CONTEXT_FUNC)
#define IVF_LOG_ERROR(msg) ivf::logError(msg, IVF_LOG_CONTEXT_FUNC)

// Macros for logging with file:line context
#define IVF_LOG_DEBUG_LOC(msg) ivf::logDebug(msg, IVF_LOG_CONTEXT_FILE)
#define IVF_LOG_INFO_LOC(msg) ivf::logInfo(msg, IVF_LOG_CONTEXT_FILE)
#define IVF_LOG_WARNING_LOC(msg) ivf::logWarning(msg, IVF_LOG_CONTEXT_FILE)
#define IVF_LOG_ERROR_LOC(msg) ivf::logError(msg, IVF_LOG_CONTEXT_FILE)

// Formatted logging macros with automatic context
#define IVF_LOGF_DEBUG(fmt, ...) ivf::Logger::instance()->debugfc(IVF_LOG_CONTEXT_FUNC, fmt, __VA_ARGS__)
#define IVF_LOGF_INFO(fmt, ...) ivf::Logger::instance()->infofc(IVF_LOG_CONTEXT_FUNC, fmt, __VA_ARGS__)
#define IVF_LOGF_WARNING(fmt, ...) ivf::Logger::instance()->warningfc(IVF_LOG_CONTEXT_FUNC, fmt, __VA_ARGS__)
#define IVF_LOGF_ERROR(fmt, ...) ivf::Logger::instance()->errorfc(IVF_LOG_CONTEXT_FUNC, fmt, __VA_ARGS__)
