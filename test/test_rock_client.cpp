#include "test.h"

#include "../seaice/rock/rock_server.h"
#include "../seaice/rock/rock_connection.h"

void run() {
    auto conn = seaice::RockConnection::Create("http://0.0.0.0:8080/seaice", 1000);
    conn->start();
    uint32_t sn = 0;
    while(true) {
        seaice::RockRequest::ptr req(new seaice::RockRequest);
        req->setSn(++sn);
        req->setBody("client req");
        conn->request(req);
        sleep(2);
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