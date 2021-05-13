#include "test.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

void run() {
    seaice::http::WSServer::ptr server = std::make_shared<seaice::http::WSServer>();
    auto addr = seaice::Address::LookupAnyIPAddress("[0.0.0.0:8080]");
    while(!server->bind(addr)) {
        sleep(2);
    }
    seaice::http::WSServletDispatcher::ptr sd = server->getDispatch();

    auto fun = [](seaice::http::HttpRequest::ptr req 
                    , seaice::http::WSFrameMessage::ptr message
                    , seaice::http::WSSession::ptr session) {
        session->sendMessage(message);
    };

    sd->addServlet("/seaice", fun);

    while(!server->bind(addr)) {
        SEAICE_LOG_ERROR(logger) << "bind " << *addr << "fail";
        sleep(2);
    }
    server->start();
}

int main(int argc, char const *argv[])
{
    seaice::IOManager iom(2);
    iom.start();
    iom.schedule(run);
    iom.stop();
    return 0;
}