#include <iostream>
#include "yaml-cpp/yaml.h"
#include "../seaice/log.h"
#include "../seaice/fiber.h"
#include "../seaice/iomanager.h"
#include "../seaice/address.h"
#include "../seaice/socket.h"
#include <unistd.h>
#include <ucontext.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "../seaice/timer.h"
#include "../seaice/hook.h"
#include "ipcdefv01.h"
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <type_traits>
#include "ipcdefv01.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("seaice");

using namespace std;
using namespace seaice;
void test(){
    seaice::IPAddress::ptr addr = seaice::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        SEAICE_LOG_DEBUG(logger) << "get address: " << addr->toString();
    } else {
        SEAICE_LOG_ERROR(logger) << "get address failed";
    }
    seaice::Socket::ptr socket = seaice::Socket::CreateTCP(addr);
    if(!socket->connect(addr)) {
        SEAICE_LOG_DEBUG(logger) << "connect " << addr->toString() <<" FAILED";
    } else {
        SEAICE_LOG_DEBUG(logger) << "connect " << addr->toString() <<" SUCCESS";
    }
}

void test_localhost() {
    const char* ip = "127.0.1.1";
    int port = 9988;
    IPv4Address::ptr addr = IPv4Address::Create(ip, port);
    seaice::Socket::ptr socket = seaice::Socket::CreateTCP(addr);
    if(!socket->connect(addr)) {
        SEAICE_LOG_DEBUG(logger) << "connect " << addr->toString() <<" FAILED";
    } else {
        SEAICE_LOG_DEBUG(logger) << "connect " << addr->toString() <<" SUCCESS";
    }

    socket->setSendTimeout(5000);
    socket->setRecvTimeout(5000);

    LogIn login;
    strcpy(login.username, "seaicelin");
    strcpy(login.password, "12344566d");

    int n = socket->send((const char*)&login, login.hdr.len, 0);
    SEAICE_LOG_DEBUG(logger) << "n = " << n;

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    n = socket->recv(buf, sizeof(buf), 0);
    SEAICE_LOG_DEBUG(logger) << "n = " << n << " buf = " <<
            buf;

    SEAICE_LOG_DEBUG(logger) << "test host end";
}

int main() {
    seaice::set_hook_enable(true);
    seaice::IOManager iom(1);
    iom.start();
    iom.schedule(&test_localhost);
    iom.stop();
    return 0;
}