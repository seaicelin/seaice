#ifndef _SEAICE_CONFIG_H_
#define _SEAICE_CONFIG_H_

//#include "Log.h"
#include <vector>
#include <string>
#include <memory>

namespace seaice {

class Logger;
class LogAppender;
class LogFormatter;

class Formatter_Yaml {
public:
    typedef std::shared_ptr<Formatter_Yaml> ptr;

    Formatter_Yaml() {}
    ~Formatter_Yaml() {}
    std::shared_ptr<LogFormatter> getFormatter();

    std::string m_pattern;
};

class Appender_Yaml {
public:
    typedef std::shared_ptr<Appender_Yaml> ptr;

    Appender_Yaml() {}
    ~Appender_Yaml() {}

    void setFormatter(Formatter_Yaml::ptr formatter) {
        m_formatter = formatter;
    }
    std::shared_ptr<LogAppender> getAppender();

    int m_type;
    Formatter_Yaml::ptr m_formatter;
};

class Logger_Yaml {
public:
    typedef std::shared_ptr<Logger_Yaml> ptr;

    Logger_Yaml() {}
    ~Logger_Yaml() {}

    void addAppender(Appender_Yaml::ptr appender) {
        m_appenders.push_back(appender);
    }
    std::shared_ptr<Logger> getLogger();

    int m_id;
    std::string m_level;
    std::string m_name;
    std::vector<Appender_Yaml::ptr> m_appenders;
    Formatter_Yaml::ptr m_formatter;
};

class Config {
public:
    typedef std::shared_ptr<Config> ptr;

    Config();
    ~Config();

public:
    std::vector<std::shared_ptr<Logger> > m_loggers;
};

}

#endif