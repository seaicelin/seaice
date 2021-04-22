#include "config.h"
#include "yaml-cpp/yaml.h"
#include "log.h"

using namespace std;
using namespace seaice;

std::shared_ptr<LogFormatter> Formatter_Yaml::getFormatter() {
    LogFormatter::ptr formatterPtr = LogFormatter::ptr(new LogFormatter(m_pattern));
    return formatterPtr;
}

std::shared_ptr<LogAppender> Appender_Yaml::getAppender() {
    LogAppender::ptr appenderPtr;
    if(m_type == 0) {
        appenderPtr = LogAppender::ptr(new StdoutLogAppender);
    } else if(m_type == 1) {
        appenderPtr = LogAppender::ptr(new FileLogAppender);
    }
    appenderPtr->setFormatter(m_formatter->getFormatter());
    return appenderPtr;
}

std::shared_ptr<Logger> Logger_Yaml::getLogger() {
    Logger::ptr loggerPtr = Logger::ptr(new Logger);
    loggerPtr->setName(m_name);
    loggerPtr->setLevel(LogLevel::fromString(m_level));
    for(auto appender : m_appenders) {
        loggerPtr->setAppender(appender->getAppender());
    }
    loggerPtr->setFormatter(m_formatter->getFormatter());
    return loggerPtr;
}

Config::Config() {
    YAML::Node config = YAML::LoadFile("../seaice/config/logConfig.yaml");

    YAML::Node loggers = config["loggers"];
    cout << "loggers size = " << loggers.size() <<endl;

    for(std::size_t i = 0; i < loggers.size(); i++)
    {
        Logger_Yaml::ptr loggerPtr = Logger_Yaml::ptr(new Logger_Yaml);
        YAML::Node logger = loggers[i];

        loggerPtr->m_id = logger["id"].as<int>();
        loggerPtr->m_level = logger["level"].as<string>();
        loggerPtr->m_name = logger["name"].as<string>();

        YAML::Node appenders = logger["appenders"];
        for(std::size_t j = 0; j < appenders.size(); j++)
        {
            YAML::Node appender = appenders[j];
            Formatter_Yaml::ptr formatterPtr = Formatter_Yaml::ptr(new Formatter_Yaml);
            formatterPtr->m_pattern = appender["formatter"]["pattern"].as<string>();

            Appender_Yaml::ptr appenderPtr = Appender_Yaml::ptr(new Appender_Yaml);
            appenderPtr->m_type = appender["type"].as<int>();
            appenderPtr->setFormatter(formatterPtr);
            loggerPtr->addAppender(appenderPtr);
        }
        Formatter_Yaml::ptr formatterPtr = Formatter_Yaml::ptr(new Formatter_Yaml);
        formatterPtr->m_pattern = logger["formatter"]["pattern"].as<string>();
        loggerPtr->m_formatter = formatterPtr;
        m_loggers.push_back(loggerPtr->getLogger());
    }
}

Config::~Config() {
}