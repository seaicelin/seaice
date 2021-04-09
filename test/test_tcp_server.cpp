#include "test.h"
#include "../seaice/tcpserver.h"
#include "../seaice/address.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("seaice");

void run() {
    auto addr = seaice::Address::LookupAny("[0.0.0.0:8080]");
    auto addr2 = seaice::Address::LookupAny("[0.0.0.0]");
    auto addr3 = seaice::Address::LookupAny("0.0.0.0:8080");
    auto addr4 = seaice::Address::LookupAny("0.0.0.0");
    auto addr5 = seaice::UnixAddress::ptr(new seaice::UnixAddress("/tmp/unix_address"));
    SEAICE_LOG_DEBUG(logger) << "addr = " << *addr;
    SEAICE_LOG_DEBUG(logger) << "addr2 = " << *addr2;
    SEAICE_LOG_DEBUG(logger) << "addr3 = " << *addr3;
    SEAICE_LOG_DEBUG(logger) << "addr4 = " << *addr4;
    SEAICE_LOG_DEBUG(logger) << "addr5 = " << *addr5;

    seaice::TcpServer::ptr tcp_server(new seaice::TcpServer);
    std::vector<seaice::Address::ptr> fails;
    std::vector<seaice::Address::ptr> addrs;
    addrs.push_back(addr);
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
}

int main() {
    seaice::IOManager iom(2);
    iom.start();
    iom.schedule(run);
    iom.stop();
    return 0;
}