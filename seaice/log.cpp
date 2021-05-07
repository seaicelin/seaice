#include <iostream>
#include <algorithm>
#include "log.h"
#include "hook.h"
#include "config2.h"

using namespace std;
namespace seaice{

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
    if(event->getLevel() >= m_level) {
        MutexType::Lock lock(m_mutex);
        for(auto appender : m_appenders) {
            appender->format(event);
        }
    }
}

void Logger::clearAppenders() {
    m_appenders.clear();
}

void Logger::setAppender(LogAppender::ptr appender) {
    auto it = find(m_appenders.begin(), m_appenders.end(), appender);
    if(it != m_appenders.end()) {
        return;
    }
    m_appenders.push_back(appender);
}

void Logger::setFormatter(std::shared_ptr<LogFormatter> formatter) {
    m_formatter = formatter;
    for(auto it : m_appenders) {
        it->setFormatter(formatter);
    }
}

void Logger::setFormatter(const std::string& pattern) {
    LogFormatter::ptr fmt(new LogFormatter(pattern));
    if(fmt && !fmt->isError()) {
        setFormatter(fmt);
    }
}

std::string Logger::toYamlString() const {
    YAML::Node node;
    node["id"] = m_id;
    node["name"] = m_name;
    if(m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::toString(m_level);
    }
    if(m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    for(auto& i : m_appenders) {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void StdoutLogAppender::format(LogEvent::ptr event) {
    stringstream oss;
    m_formatter->format(oss, event);
    cout << oss.str() << endl;
}

std::string StdoutLogAppender::toYamlString() {
    //MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::toString(m_level);
    }
    if(m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    stringstream ss;
    ss << node;
    return ss.str();
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

std::string FileLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_path;
    if(m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::toString(m_level);
    }
    if(m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    stringstream ss;
    ss << node;
    return ss.str();
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
        cout<<"formatter init error: state = " << state << " str = " << m_pattern[index]
            << " index = " << index << endl;
        m_error = true;
        return;
        //throw std::logic_error("logic error");;
    }

    for(auto it : items)
    {
        m_items.push_back(getFormatterItem(it.first, it.second));
    }
}

struct LogAppenderDefine {
    int type; //2: std::out, 1: File
    std::string formatter;
    std::string file;
    LogLevel::Level level = LogLevel::UNKNOW;

    bool operator==(const LogAppenderDefine& rhs) const {
        return type == rhs.type
            && formatter == rhs.formatter
            && file == rhs.file
            && level == rhs.level;
    }
};

struct LoggerDefine {
    int id;
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::vector<LogAppenderDefine> appenders;
    std::string formatter;
    bool operator==(const LoggerDefine& rhs) const {
        return name == rhs.name
            && id == rhs.id
            && level == rhs.level
            && formatter == rhs.formatter
            && appenders == rhs.appenders;
    }

    bool operator<(const LoggerDefine& rhs) const {
        return name < rhs.name;
    }

    bool isValid() const {
        return !name.empty();
    }
};

template<>
class LexicalCast<std::string, LoggerDefine> {
public:
    LoggerDefine operator()(const string& v) {
        YAML::Node node = YAML::Load(v);
        LoggerDefine ld;
        if(!node["name"].IsDefined()) {
            std::cout << "log config error: name is null, "
                << node << endl;
            throw std::logic_error("log config name is null");
        }
        ld.name = node["name"].as<std::string>();
        if(!node["id"].IsDefined()) {
            std::cout << "log config error: id is null, "
                << node << endl;
            throw std::logic_error("log config id is null");
        }
        ld.id = node["id"].as<int>();
        if(node["level"].IsDefined()) {
            std::string level = node["level"].as<std::string>();
            //std::cout <<  << std::endl;
            ld.level = LogLevel::fromString(level);
        }
        if(node["formatter"].IsDefined()) {
            ld.formatter = node["formatter"].as<std::string>();
        }
        if(node["appenders"].IsDefined()) {
            for(size_t i = 0; i < node["appenders"].size(); ++i) {
                LogAppenderDefine lad;
                auto a = node["appenders"][i];
                if(!a["type"].IsDefined()) {
                    std::cout << "log config error: appender type is null,"
                        << a << std::endl;
                    continue;
                }
                std::string type = a["type"].as<std::string>();
                if(type == "FileLogAppender") {
                    lad.type = 1;
                    lad.file = a["file"].as<std::string>();
                    if(a["formatter"].IsDefined()) {
                        lad.formatter = a["formatter"].as<std::string>();
                    }
                } else if(type == "StdoutLogAppender") {
                    lad.type = 2;
                    if(a["formatter"].IsDefined()) {
                        lad.formatter = a["formatter"].as<std::string>();
                    }
                } else {
                    std::cout << "log config error: appender type is invalid, "
                        << a << std::endl;
                        continue;
                }
                ld.appenders.push_back(lad);
            }
        }
        return ld;
    }
};

template<>
class LexicalCast<LoggerDefine, std::string> {
public:
    std::string operator()(const LoggerDefine& v) {
        YAML::Node node;
        node["id"] = v.id;
        node["name"] = v.name;
        if(v.level != LogLevel::UNKNOW) {
            node["level"] = LogLevel::toString(v.level);
        }
        if(!v.formatter.empty()) {
            node["formatter"] = v.formatter;
        }
        for(auto& a : v.appenders) {
            YAML::Node na;
            if(a.type == 1) {
                na["type"] = "FileLogAppender";
                na["file"] = a.file;
            } else if(a.type == 2) {
                na["type"] = "StdoutLogAppender";
            }
            if(!a.formatter.empty()) {
                na["formatter"] = a.formatter;
            }
            node["appenders"].push_back(na);
        }
        stringstream ss;
        ss << node;
        return ss.str();
    }
};


seaice::ConfigVar<std::set<LoggerDefine> >::ptr g_log_defines =
    seaice::Config2::Lookup("loggers", std::set<LoggerDefine>(), "loggers config");

struct LoggerInit{
    LoggerInit() {

        g_log_defines->addListener([](const std::set<LoggerDefine>& old_val,
                const std::set<LoggerDefine>& new_val) {
            std::cout << "on_logger_conf_changed" << std::endl;
            for(auto& i : new_val) {
                auto it = old_val.find(i);
                seaice::Logger::ptr logger;
                if(it != old_val.end() && (*it) == i) {
                    continue;
                }
                if(nullptr == LoggerMgr::getInstance()->findLogger(i.name)) {
                    logger = Logger::ptr(new Logger);
                }
                logger->setId(i.id);
                logger->setName(i.name);
                logger->setLevel(i.level);
                if(!i.formatter.empty()) {
                    LogFormatter::ptr fmt(new LogFormatter(i.formatter));
                    logger->setFormatter(fmt);
                }
                logger->clearAppenders();
                for(auto& a : i.appenders) {
                    seaice::LogAppender::ptr ap;
                    if(a.type == 1) {
                        ap.reset(new FileLogAppender(a.file));
                    } else if(a.type == 2) {
                        ap.reset(new StdoutLogAppender);
                    }
                    ap->setLevel(a.level);
                    if(!a.formatter.empty()){
                        LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                        if(fmt && !fmt->isError()) {
                            ap->setFormatter(fmt);
                        }
                    }
                    logger->setAppender(ap);
                }
                LoggerMgr::getInstance()->addLogger(logger);
            }
            for(auto& i : old_val) {
                auto it = new_val.find(i);
                if(it == new_val.end()) {
                    LoggerMgr::getInstance()->delLogger(i.name);
                }
            }
        });
        //std::cout<<g_log_defines->toString();
    }
};

static LoggerInit __log_init;

LoggerMgr::LoggerMgr() {
//        loadLogConfig();
    seaice::init_hook();

    m_logger = Logger::ptr(new Logger);
    m_logger->setAppender(LogAppender::ptr(new StdoutLogAppender));
    m_logger->setAppender(LogAppender::ptr(new FileLogAppender));
    YAML::Node root = YAML::LoadFile("/media/sf_Iceserver/vsclient/seaice/build/bin/conf/log.yml");
    seaice::Config2::LoadFromYaml(root);
    //std::cout<<"loggerMgr"<<std::endl;
}

LoggerMgr::~LoggerMgr() {
#ifdef DEBUG_DESTROY
    std::cout<<"~LoggerMgr"<<std::endl;
#endif
}

LoggerMgr::ptr LoggerMgr::getInstance() {
    static LoggerMgr::ptr s_instance = nullptr;
    if(s_instance == nullptr) {
        s_instance = ptr(new LoggerMgr);
    }
    return s_instance;
}

void LoggerMgr::addLogger(Logger::ptr logger){
    m_map[logger->getName()] = logger;
}

void LoggerMgr::delLogger(std::string name) {
    m_map.erase(name);
}

Logger::ptr LoggerMgr::findLogger(string name) {
    auto it = m_map.find(name);
    if(it != m_map.end()) {
        return (Logger::ptr)it->second;
    }
    return nullptr;
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
    assert(m_logger);
    return m_logger;
}

void LoggerMgr::loadLogConfig() {

    seaice::Config::ptr logConfigPtr = seaice::Config::ptr(new seaice::Config);
    if(!logConfigPtr->m_loggers.empty()) {
        vector<Logger::ptr> loggers;
        loggers.swap(logConfigPtr->m_loggers);
        for(auto logger : loggers) {
            m_map[logger->getName()] = logger;
        }
    }
}

std::string LoggerMgr::toYamlString() {
    YAML::Node node;
    for(auto& i : m_map) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

}