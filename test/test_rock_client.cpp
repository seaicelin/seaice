#include "test.h"

#include "../seaice/rock/rock_server.h"
#include "../seaice/rock/rock_connection.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

void run() {
    auto conn = seaice::RockConnection::Create("http://0.0.0.0:8080/seaice");
    if(conn == nullptr) {
        SEAICE_LOG_ERROR(logger) << "can not create RockConnection!";
        return;
    }
    conn->start();
    uint32_t sn = 0;
    seaice::Timer::ptr timer(new seaice::Timer(4000, [conn, &sn] () {
        seaice::RockRequest::ptr req(new seaice::RockRequest);
        req->setSn(++sn);
        req->setBody("client req");
        conn->request(req);
    }, true));

    seaice::IOManager* iom = seaice::IOManager::GetThis();
    iom->addTimer(timer);
}

int main(int argc, char const *argv[])
{
    seaice::IOManager iom(2);
    iom.start();
    iom.schedule(run);
    iom.stop();
    return 0;
}