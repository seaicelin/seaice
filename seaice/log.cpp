#include <iostream>
#include <algorithm>
#include "log.h"
#include "hook.h"

using namespace std;
using namespace seaice;

void LogEvent::format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    format(fmt, ap);
    va_end(ap);
}

void LogEvent::format(const char* fmt, va_list ap) {
    char* buf = nullptr;
    auto len = vasprintf(&buf, fmt, ap);
    if(len != -1) {
        m_ss << string(buf, len);
        free(buf);
    }
}

void Logger::log(LogEvent::ptr event) {
    if(event->getLevel() >= m_logLevel) {
        MutexType::Lock lock(m_mutex);
        for(auto appender : m_appenders) {
            appender->format(event);
        }
    }
}

void Logger::setAppender(LogAppender::ptr appender) {
    auto it = find(m_appenders.begin(), m_appenders.end(), appender);
    if(it != m_appenders.end()) {
        return;
    }
    m_appenders.push_back(appender);
}

void Logger::setFormatter(std::shared_ptr<LogFormatter> formatter) {
    for(auto it : m_appenders) {
        it->setFormatter(formatter);
    }
}

void StdoutLogAppender::format(LogEvent::ptr event) {
    stringstream oss;
    m_formatter->format(oss, event);
    cout << oss.str() << endl;
}

void FileLogAppender::format(LogEvent::ptr event) {
    MutexType::Lock lock(m_mutex);
    if(m_ofstream.is_open()) {
        stringstream oss;
        m_formatter->format(oss, event);
        m_ofstream << oss.str() << endl;
        //m_ofstream.flush();
        //m_ofstream.close();
    } else {
        std::cout << "FileLogAppender format open fail!" << std::endl;
    }
}

void LogFormatter::format(stringstream& oss, LogEvent::ptr event) {
    for(auto item : m_items) {
        item->format(oss, event);
    }
}

class ContentLogFormatterItem : public LogFormatter::FormaterItem {
public:
    typedef std::shared_ptr<ContentLogFormatterItem> ptr;

    ContentLogFormatterItem(const string str = "") {}
    ~ContentLogFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << event->getContent();
    }
};

class LevelLogFormatterItem : public LogFormatter::FormaterItem {
public:
    typedef std::shared_ptr<LevelLogFormatterItem> ptr;

    LevelLogFormatterItem(const string str = "") {}
    ~LevelLogFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << LogLevel::toString(event->getLevel());
    }
};

class ElaspTimeLogFormatterItem : public LogFormatter::FormaterItem {
public:
    typedef std::shared_ptr<ElaspTimeLogFormatterItem> ptr;

    ElaspTimeLogFormatterItem(const string str = "") {}
    ~ElaspTimeLogFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << event->getElaspTime();
    }
};

class LoggerNameFormatterItem : public LogFormatter::FormaterItem
{
public:
    typedef std::shared_ptr<LoggerNameFormatterItem> ptr;

    LoggerNameFormatterItem(const string str = "") {}
    ~LoggerNameFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << event->getLoggerName();
    }
};

class ThreadIdFormatterItem : public LogFormatter::FormaterItem {
public:
    ThreadIdFormatterItem(const string str = "") {}
    ~ThreadIdFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << event->getThreadId();
    }
};

class NewLineFormatterItem : public LogFormatter::FormaterItem
{
public:
    NewLineFormatterItem(const string str = "") {}
    ~NewLineFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << std::endl;
    }
};

class DateFormatterItem : public LogFormatter::FormaterItem {
public:
    DateFormatterItem(const string str = "")
    : m_str(str) {
    }
    ~DateFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        time_t time = event->getTime();
        tm* t = localtime(&time);
        char buffer[32];
        strftime(buffer, 32, m_str.c_str(), t);
        oss << buffer;
    }
private:
    string m_str;
};

class FileNameFormatterItem : public LogFormatter::FormaterItem {
public:
    FileNameFormatterItem(const string str = "") {}
    ~FileNameFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << event->getFileName();
    }
};

class LineNumberFormatterItem : public LogFormatter::FormaterItem {
public:
    LineNumberFormatterItem(const string str = "") {}
    ~LineNumberFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << event->getLineNumber();
        uint32_t len = event->getFileName().length() + 
            std::to_string(event->getLineNumber()).length();
        //char space = ' ';
        //std::string str = string(space, 100 - len);
        len = 75 - len;
        while(len > 0) {
            oss << " ";
            --len;
        }
    }
};

class TabFormatterItem : public LogFormatter::FormaterItem {
public:
    TabFormatterItem(const string str = "") {}
    ~TabFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << "    ";
    }
};

class FiberIdFormatterItem : public LogFormatter::FormaterItem {
public:
    FiberIdFormatterItem(const string str = "") {}
    ~FiberIdFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << event->getFiberId();
    }
};

class ThreadNameFormatterItem : public LogFormatter::FormaterItem {
public:
    ThreadNameFormatterItem(const string str = "") {}
    ~ThreadNameFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << event->getThreadName();
    }
};

class StringFormatterItem : public LogFormatter::FormaterItem {
public:
    StringFormatterItem(const string str = "")
    : m_str(str) {}
    ~StringFormatterItem() {}

    void format(stringstream& oss, LogEvent::ptr event) override {
        oss << m_str;
    }
private:
    string m_str;
};

LogFormatter::FormaterItem::ptr LogFormatter::getFormatterItem(char name, string arg) {
#define XX(item) LogFormatter::FormaterItem::ptr(new item(arg))
    switch (name) {
        case 'm' : return XX(ContentLogFormatterItem);
        case 'd' : return XX(DateFormatterItem);
        case 't' : return XX(ThreadIdFormatterItem);
        case 'T' : return XX(TabFormatterItem);
        case 'N' : return XX(ThreadNameFormatterItem);
        case 'f' : return XX(FileNameFormatterItem);
        case 'p' : return XX(LevelLogFormatterItem);
        case 'l' : return XX(LineNumberFormatterItem);
        case 'c' : return XX(LoggerNameFormatterItem);
        case 'n' : return XX(NewLineFormatterItem);
        case 'F' : return XX(FiberIdFormatterItem);
        case ':' : 
        case '[' : 
        case ']' : return XX(StringFormatterItem);
        default:
            return XX(StringFormatterItem);
    }
#undef XX
}

void LogFormatter::init() {
    if(m_pattern.empty()) {
        m_pattern = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
        std::cout << "m_pattern is empty, use default value" << std::endl;
    }

    int state = 0;
    int strLen = m_pattern.size();
    int index = 0;
    string STR = "ABCDEFGHIJKLMNOKQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::vector<pair<char, string> > items;
    while(index < strLen) {
        char c = m_pattern[index];
        string arg = "";
        if(state == 0) {
            if(c == '%') {
                state = 1;
            } else if(c == '[' || c == ']' || c == ':') {
                state = 0;
                arg = c;
                items.push_back(make_pair(c, arg));
            } else {
                break;
            }
        } else if(state == 1) {
            if(STR.find(m_pattern[index]) == string::npos) {
                std::cout << "init error" << std::endl;
                break;
            }
            int start, end;
            start = end = index + 1;
            if(start < strLen && m_pattern[start] == '{')
            {
                string::size_type pos = m_pattern.find("}", start);
                if(pos == string::npos) {
                    break;
                }
                index = pos;
                arg = m_pattern.substr(start+1, pos - start - 1);
                //cout<<arg<<endl;
            }
            state = 0;
            items.push_back(make_pair(c, arg));
        }
        index++;
    }
    if(state != 0 || index < strLen) {
        cout<<"state = " << state << " str = " << m_pattern[index]
            << " index = " << index << endl;
        throw std::logic_error("logic error");;
    }

    for(auto it : items)
    {
        m_items.push_back(getFormatterItem(it.first, it.second));
    }
}

Logger::ptr LoggerMgr::LookupLogger(string loggerName) {
    auto it = m_map.find(loggerName);
    if(it != m_map.end())
    {
        return (Logger::ptr)it->second;
    }
    return getDefaultLogger();
}

Logger::ptr LoggerMgr::getDefaultLogger() {
    return m_logger;
}

void LoggerMgr::loadLogConfig() {

    seaice::init_hook();

    m_logger = Logger::ptr(new Logger);
    m_logger->setAppender(LogAppender::ptr(new StdoutLogAppender));
    m_logger->setAppender(LogAppender::ptr(new FileLogAppender));

    seaice::Config::ptr logConfigPtr = seaice::Config::ptr(new seaice::Config);
    if(!logConfigPtr->m_loggers.empty()) {
        vector<Logger::ptr> loggers;
        loggers.swap(logConfigPtr->m_loggers);
        for(auto logger : loggers) {
            m_map[logger->getName()] = logger;
        }
    }
}