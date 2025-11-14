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
    bool m_showContext;

    std::string levelToString(LogLevel level) const;
    std::string getTimestamp() const;
    void writeLog(LogLevel level, const std::string &message, const std::string &context = "");
    
    // Helper function to build formatted strings
    template<typename... Args>
    std::string formatMessage(const std::string& format, Args... args) const {
        std::ostringstream oss;
        formatMessageImpl(oss, format, args...);
        return oss.str();
    }
    
    // Base case for recursion
    void formatMessageImpl(std::ostringstream& oss, const std::string& format) const {
        oss << format;
    }
    
    // Recursive case - replace {} with argument
    template<typename T, typename... Args>
    void formatMessageImpl(std::ostringstream& oss, const std::string& format, T value, Args... args) const {
        size_t pos = format.find("{}");
        if (pos != std::string::npos) {
            oss << format.substr(0, pos) << value;
            formatMessageImpl(oss, format.substr(pos + 2), args...);
        } else {
            oss << format;
        }
    }

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

    void debug(const std::string &message, const std::string &context = "");
    void info(const std::string &message, const std::string &context = "");
    void warning(const std::string &message, const std::string &context = "");
    void error(const std::string &message, const std::string &context = "");
    
    // Formatted logging methods (without context)
    template<typename... Args>
    void debugf(const std::string& format, Args... args) {
        writeLog(LogLevel::Debug, formatMessage(format, args...), "");
    }
    
    template<typename... Args>
    void infof(const std::string& format, Args... args) {
        writeLog(LogLevel::Info, formatMessage(format, args...), "");
    }
    
    template<typename... Args>
    void warningf(const std::string& format, Args... args) {
        writeLog(LogLevel::Warning, formatMessage(format, args...), "");
    }
    
    template<typename... Args>
    void errorf(const std::string& format, Args... args) {
        writeLog(LogLevel::Error, formatMessage(format, args...), "");
    }
    
    // Formatted logging methods with context (using 'c' suffix for clarity)
    template<typename... Args>
    void debugfc(const std::string& context, const std::string& format, Args... args) {
        writeLog(LogLevel::Debug, formatMessage(format, args...), context);
    }
    
    template<typename... Args>
    void infofc(const std::string& context, const std::string& format, Args... args) {
        writeLog(LogLevel::Info, formatMessage(format, args...), context);
    }
    
    template<typename... Args>
    void warningfc(const std::string& context, const std::string& format, Args... args) {
        writeLog(LogLevel::Warning, formatMessage(format, args...), context);
    }
    
    template<typename... Args>
    void errorfc(const std::string& context, const std::string& format, Args... args) {
        writeLog(LogLevel::Error, formatMessage(format, args...), context);
    }
};

typedef Logger *LoggerPtr;

void logDebug(const std::string &message, const std::string &context = "");
void logInfo(const std::string &message, const std::string &context = "");
void logWarning(const std::string &message, const std::string &context = "");
void logError(const std::string &message, const std::string &context = "");

// Formatted global logging functions (without context)
template<typename... Args>
void logDebugf(const std::string& format, Args... args) {
    Logger::instance()->debugf(format, args...);
}

template<typename... Args>
void logInfof(const std::string& format, Args... args) {
    Logger::instance()->infof(format, args...);
}

template<typename... Args>
void logWarningf(const std::string& format, Args... args) {
    Logger::instance()->warningf(format, args...);
}

template<typename... Args>
void logErrorf(const std::string& format, Args... args) {
    Logger::instance()->errorf(format, args...);
}

// Formatted global logging functions with context
template<typename... Args>
void logDebugfc(const std::string& context, const std::string& format, Args... args) {
    Logger::instance()->debugfc(context, format, args...);
}

template<typename... Args>
void logInfofc(const std::string& context, const std::string& format, Args... args) {
    Logger::instance()->infofc(context, format, args...);
}

template<typename... Args>
void logWarningfc(const std::string& context, const std::string& format, Args... args) {
    Logger::instance()->warningfc(context, format, args...);
}

template<typename... Args>
void logErrorfc(const std::string& context, const std::string& format, Args... args) {
    Logger::instance()->errorfc(context, format, args...);
}

void setLogLevel(LogLevel level);
void setConsoleOutput(bool enable);
bool setFileOutput(const std::string &filePath);
void disableFileOutput();
void setShowTimestamp(bool enable);
void setShowContext(bool enable);

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
