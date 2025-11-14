#include <ivf/logger.h>
#include <chrono>
#include <ctime>
#include <iomanip>

using namespace ivf;
using namespace std;

Logger *Logger::m_instance = nullptr;

Logger::Logger() : m_logLevel(LogLevel::Info), m_consoleOutput(true), m_fileOutput(false), m_showTimestamp(true), m_showContext(true) {}

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

std::string Logger::levelToString(LogLevel level) const { 
    switch (level) { 
        case LogLevel::Debug: return "DEBUG"; 
        case LogLevel::Info: return "INFO"; 
        case LogLevel::Warning: return "WARNING"; 
        case LogLevel::Error: return "ERROR"; 
        case LogLevel::None: return "NONE"; 
        default: return "UNKNOWN"; 
    } 
}

std::string Logger::getTimestamp() const { 
    auto now = std::chrono::system_clock::now(); 
    auto timeT = std::chrono::system_clock::to_time_t(now); 
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; 
    std::stringstream ss; 
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S"); 
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count(); 
    return ss.str(); 
}

void Logger::writeLog(LogLevel level, const std::string &message, const std::string &context) { 
    if (m_logLevel == LogLevel::None || level < m_logLevel) 
        return; 
        
    std::stringstream logMessage; 
    if (m_showTimestamp) 
        logMessage << "[" << getTimestamp() << "] "; 
        
    logMessage << "[" << levelToString(level) << "] ";
    
    if (m_showContext && !context.empty())
        logMessage << "[" << context << "] ";
    
    logMessage << message; 
    std::string finalMessage = logMessage.str(); 
    
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