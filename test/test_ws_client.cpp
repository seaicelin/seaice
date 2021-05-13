#include "test.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

void run() {
    auto rt = seaice::http::WSConnection::Create("http://0.0.0.0:8080/seaice", 10000);
    if(!rt.second) {
        SEAICE_LOG_ERROR(logger) << "Create ws connection failed";
        return;
    }
    auto conn = rt.second;
    while(true) {
        conn->sendMessage("hello seaice");
        auto msg = conn->recvMessage();
        if(!msg) {
            break;
        }
        SEAICE_LOG_DEBUG(logger) << "msg opcode = " << msg->getOpcode()
            << " data = " << msg->getData();
        sleep(5);
    }
}

int main(int argc, char const *argv[])
{
    seaice::IOManager iom(2);
    iom.start();
    iom.schedule(run);
    iom.stop();
    return 0;
}