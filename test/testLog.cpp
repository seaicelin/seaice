#include <iostream>
#include "../seaice/log.h"

enum T{
    A = 0,
    B = 2
};

T testVal(T a) {
    T b = T::A;
    return b;
}

int main(int argc, char** argv)
{
    /*
    seaice::Logger::ptr logger = seaice::LoggerMgr::getDefaultLogger();
    (seaice::LogEventWrapper( \
            std::shared_ptr<seaice::LogEvent> \
            (new seaice::LogEvent(__LINE__, __FILE__, seaice::LogLevel::DEBUG, \
            0, "thread", 0, 0, 0)), logger).getSs());
    
    using namespace seaice;
    seaice::Logger::ptr m_logger = seaice::Logger::ptr(new Logger);
    LogAppender::ptr appender = LogAppender::ptr(new StdoutLogAppender);
    m_logger->setAppender(appender);

    LogEvent::ptr event = LogEvent::ptr(new LogEvent(123, "main", 
        seaice::LogLevel::DEBUG, 0, "thread", 0, 0, 0));

    LogEventWrapper(event, m_logger).getSs();

*/
    //seaice::Logger::ptr m_logger = seaice::LoggerMgr::getInstance()->getLogger();
    SEAICE_LOG_DEBUG(SEAICE_LOGGER("seaice")) << "HELLO WORLD";
    SEAICE_LOG_DEBUG(SEAICE_LOGGER("seaice")) << "HELLO seaice";

    SEAICE_LOG_DEBUG(SEAICE_LOGGER("root")) << "HELLO who am i";

    SEAICE_LOG_FMT_DEBUG(SEAICE_LOGGER("system"), "%d, %s", 1234, "seaice");


    int a = 1;
    a = testVal((T)a);



    return 0;
}