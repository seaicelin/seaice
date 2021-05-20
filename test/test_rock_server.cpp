#include "test.h"

#include "../seaice/rock/rock_server.h"

void run() {
    seaice::RockServer::ptr server(new seaice::RockServer());
    auto addr = seaice::Address::LookupAny("[0.0.0.0:8080]");
    std::vector<seaice::Address::ptr> fails;
    std::vector<seaice::Address::ptr> addrs;
    addrs.push_back(addr);
    while(!server->bind(addrs, fails)) {
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