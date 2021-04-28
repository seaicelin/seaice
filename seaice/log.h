#ifndef _SEAICE_LOG_H_
#define _SEAICE_LOG_H_

#include <string>
#include <stdint.h>
#include <sstream>
#include <memory>
#include <list>
#include <vector>
#include <map>
#include <ctime>
#include <stdarg.h>
#include <fstream>
#include <iostream>
#include "utils.h"
#include "config.h"
#include "thread.h"
#include "mutex.h"
#include "macro.h"

#define SEAICE_LOGGER(name) seaice::LoggerMgr::getInstance()->LookupLogger(name)

#define SEAICE_LOG_FMT(logger, level, fmt, ...) seaice::LogEventWrapper( \
            std::shared_ptr<seaice::LogEvent> \
            (new seaice::LogEvent(__LINE__, __FILE__, level, \
            seaice::utils::getThreadId(), seaice::Thread::GetName(), 0, ::time(nullptr), 0)), logger).getEvent()->format(fmt, __VA_ARGS__)

#define SEAICE_LOG(logger, level) seaice::LogEventWrapper( \
            std::shared_ptr<seaice::LogEvent> \
            (new seaice::LogEvent(__LINE__, __FILE__, level, \
            seaice::utils::getThreadId(), seaice::Thread::GetName(), 0, ::time(nullptr), 0)), logger).getSs()

#define SEAICE_LOG_DEBUG(logger) SEAICE_LOG(logger, seaice::LogLevel::DEBUG)

#define SEAICE_LOG_INFO(logger) SEAICE_LOG(logger, seaice::LogLevel::INFO)

#define SEAICE_LOG_WARN(logger) SEAICE_LOG(logger, seaice::LogLevel::WARN)

#define SEAICE_LOG_ERROR(logger) SEAICE_LOG(logger, seaice::LogLevel::ERROR)

#define SEAICE_LOG_FATAL(logger) SEAICE_LOG(logger, seaice::LogLevel::FATAL)

#define SEAICE_LOG_FMT_DEBUG(logger, fmt, ...) \
            SEAICE_LOG_FMT(logger, seaice::LogLevel::DEBUG, fmt, __VA_ARGS__)

#define SEAICE_LOG_FMT_INFO(logger, fmt, ...) \
            SEAICE_LOG_FMT(logger, seaice::LogLevel::INFO, fmt, __VA_ARGS__)

#define SEAICE_LOG_FMT_WARN(logger, fmt, ...) \
            SEAICE_LOG_FMT(logger, seaice::LogLevel::WARN, fmt, __VA_ARGS__)

#define SEAICE_LOG_FMT_ERROR(logger, fmt, ...) \
            SEAICE_LOG_FMT(logger, seaice::LogLevel::ERROR, fmt, __VA_ARGS__)

#define SEAICE_LOG_FMT_FATAL(logger, fmt, ...) \
            SEAICE_LOG_FMT(logger, seaice::LogLevel::FATAL, fmt, __VA_ARGS__)

//#define DEBUG_DESTROY 1

namespace seaice {

using std::string;
using std::stringstream;
using std::ofstream;
//using std::shared_ptr;

class LogFormatter;
class LogAppender;
class StdoutLogAppender;

class LogLevel
{
public:
    enum Level
    {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    static string toString(Level level) {
        switch(level) {
            case UNKNOW: return "UNKNOW";
            case DEBUG: return "DEBUG";
            case INFO: return "INFO";
            case WARN: return "WARN";
            case ERROR: return "ERROR";
            case FATAL: return "FATAL";
            default: return "UNKNOW";
        }
    }
    static Level fromString(const string& str) {
#define XX(level, v) \
        if(str == #v) { \
             return LogLevel::level; \
        }
        XX(UNKNOW, unknow);
        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(UNKNOW, UNKNOW);
        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        return LogLevel::UNKNOW;
    }
#undef XX
};

class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(uint32_t line, string fileName, LogLevel::Level level,
        uint32_t threadId, string threadName, uint32_t fiberId,
        uint64_t time, uint64_t elasptime)
        : m_line(line)
        , m_fileName(fileName)
        , m_level(level)
        , m_threadId(threadId)
        , m_threadName(threadName)
        , m_fiberId(fiberId)
        , m_time(time)
        , m_elaspTime(elasptime) {
    }

    ~LogEvent() {
#ifdef DEBUG_DESTROY
         std::cout<<"~LogEvent"<<std::endl;
#endif
    }

     stringstream& getSs() {return m_ss;}
    const uint32_t getLineNumber() const {return m_line;}
    const string getFileName() const {return m_fileName;}
    const LogLevel::Level getLevel() const {return m_level;}
    const uint32_t getThreadId() const {return m_threadId;}
    const uint32_t getFiberId() const {return m_fiberId;}
    const uint64_t getTime() const {return m_time;}
    const uint64_t getElaspTime() const {return m_elaspTime;}
    const string getContent() const {return m_ss.str();}
    const string getThreadName() const {return m_threadName;}
    void setLoggerName(string loggerName) { m_loggerName = loggerName;}
    const string getLoggerName() const {return m_loggerName;}
     void format(const char* fmt, ...);
     void format(const char* fmt, va_list ap);
private:
    uint32_t m_line = 0;;
    string   m_fileName;
    LogLevel::Level m_level = LogLevel::UNKNOW;
    uint32_t m_threadId = 0;
    string m_threadName;
    uint32_t m_fiberId = 0;
    uint64_t m_time = 0;
    uint64_t m_elaspTime = 0;
    string m_content;
    string m_loggerName = "UNKNOW";
    stringstream m_ss;
};

class Logger
{
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Mutex MutexType;

    Logger(LogLevel::Level level = LogLevel::DEBUG, string name = "seaice")
        : m_level(level)
        , m_name(name) {
    }

    Logger(std::shared_ptr<LogAppender> appender) {
        m_appenders.push_back(appender);
    }
    ~Logger() {
#ifdef DEBUG_DESTROY
        std::cout<<"~Logger"<<std::endl;
#endif
    }

    void clearAppenders();
    void setAppender(std::shared_ptr<LogAppender> appender);
    void log(std::shared_ptr<LogEvent> event);
    void setFormatter(std::shared_ptr<LogFormatter> formatter);
    void setFormatter(const std::string& pattern);
    void setLevel(LogLevel::Level level) {m_level = level;}
    void setName(string name) {m_name = name;}
    void setId(int id) {m_id = id;}
    const string& getName() const { return m_name;}
    std::string toYamlString() const;
private:
    //LogEvent::ptr m_event;
    int m_id;
    LogLevel::Level m_level;
    string m_name;
    std::list<std::shared_ptr<LogAppender> > m_appenders;
    std::shared_ptr<LogFormatter> m_formatter;
    MutexType m_mutex;
};

class LogEventWrapper {
public:
    LogEventWrapper(LogEvent::ptr event, Logger::ptr logger) 
    : m_event(event)
    , m_logger(logger) {
        m_event->setLoggerName(m_logger->getName());
    }
    ~LogEventWrapper() {
        m_logger->log(m_event);
#ifdef DEBUG_DESTROY
        std::cout<<"~LogEventWrapper"<<std::endl;
#endif
    }

    LogEvent::ptr getEvent() {return m_event;}
    stringstream& getSs() {return m_event->getSs();}

private:
    LogEvent::ptr m_event;
    Logger::ptr m_logger;
};

 class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter(const string& pattern = "%d{%Y-%m-%d %H:%M:%S}%T%f:%l%T[%p]%T[%t]%T[%c]%T%m")
    : m_pattern(pattern) {
        init();
    }

    ~LogFormatter() {}

    class FormaterItem
    {
    public:
        typedef std::shared_ptr<FormaterItem> ptr;

        FormaterItem() {}
        virtual ~FormaterItem() {}
        virtual void format(stringstream& oss, LogEvent::ptr event) = 0;
    };

    FormaterItem::ptr getFormatterItem(char name, string arg);
    void init();
    void format(stringstream& oss, LogEvent::ptr event);
    std::string getPattern() const {return m_pattern;}
    bool isError() const {return m_error;}
private:
    string m_pattern;
    std::vector<FormaterItem::ptr> m_items;
    bool m_error = false;
};

class LogAppender
{
public:
    typedef std::shared_ptr<LogAppender> ptr;

    LogAppender()
        : m_level(LogLevel::UNKNOW) {
    }
    virtual ~LogAppender() {}

    void setFormatter(LogFormatter::ptr formatter) {m_formatter = formatter;}
    void setLevel(LogLevel::Level v) {m_level = v;}

    virtual void format(LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;
 protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

class StdoutLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    StdoutLogAppender() {
        m_formatter = LogFormatter::ptr(new LogFormatter);
    }
    ~StdoutLogAppender() {}

    void format(LogEvent::ptr event) override;
    std::string toYamlString() override;
};

class FileLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    typedef Mutex MutexType;

    FileLogAppender(const string& path = "seaice.log") 
    : m_path(path) {
        m_path = "/media/sf_shared/Iceserver/vsclient/seaice/build/seaice.log";
        m_formatter = LogFormatter::ptr(new LogFormatter);
        seaice::utils::createFileDir(m_path);
        MutexType::Lock lock(m_mutex);
        m_ofstream.open(m_path, std::ios::out | std::ios::in | std::ios::trunc);
        if(!m_ofstream.is_open()) {
            std::cout <<"open file fail" << std::endl;
        }
    }
    ~FileLogAppender() {
        if(m_ofstream.is_open()) {
            m_ofstream.flush();
            m_ofstream.close();
        }
    }

    void format(LogEvent::ptr event) override;
    std::string toYamlString() override;
private:
    string m_path;
    ofstream m_ofstream;
    MutexType m_mutex;
};

class LoggerMgr {
public:
    typedef std::shared_ptr<LoggerMgr> ptr;

    LoggerMgr();
    ~LoggerMgr();

    static LoggerMgr::ptr getInstance();

    Logger::ptr LookupLogger(std::string loggerName);
    Logger::ptr getDefaultLogger();
    void addLogger(Logger::ptr logger);
    void delLogger(std::string name);
    Logger::ptr findLogger(std::string name);
    std::string toYamlString();
private:
    void loadLogConfig();

private:
    std::map<string, Logger::ptr> m_map;
    Logger::ptr m_logger;
};

}
#endif
