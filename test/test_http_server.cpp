#include "test.h"
#include "../seaice/http/http_server.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

void run() {
    seaice::http::HttpServer::ptr server(new seaice::http::HttpServer);
    auto addr = seaice::Address::LookupAnyIPAddress("[0.0.0.0:8080]");
    while(!server->bind(addr)) {
        sleep(2);
    }
    server->start();
}

int main() {
    seaice::IOManager iom(2);
    iom.start();
    iom.schedule(run);
    iom.stop();
}