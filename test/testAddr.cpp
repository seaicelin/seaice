#include <iostream>
#include "yaml-cpp/yaml.h"
#include "../seaice/log.h"
#include "../seaice/fiber.h"
#include "../seaice/iomanager.h"
#include "../seaice/address.h"
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

static seaice::Logger::ptr logger = SEAICE_LOGGER("seaice");

using namespace std;

void test_getaddrinfo()
{
    int ret = -1;
    struct addrinfo* res;
    struct addrinfo hint;
    struct addrinfo* cur;

    char ipstr[16];
    bzero(&hint, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = 0;

    ret = getaddrinfo("www.baidu.com", NULL, &hint, &res);
    if(ret != 0) {
        SEAICE_LOG_ERROR(logger) << "getaddrinfo failed";
    }
    for(cur = res; cur != NULL; cur=cur->ai_next) {
        inet_ntop(AF_INET, &(((struct sockaddr_in *)(cur->ai_addr))->sin_addr), 
                ipstr, 16);
        SEAICE_LOG_DEBUG(logger) << ipstr;
    }
    freeaddrinfo(res);
}

void test_getifaddrs() {
    struct ifaddrs *ifaddr, *ifa;
    int family, s, n;
    char host[NI_MAXHOST];
    char ipstr[128];
    char netmask[128];

    if(getifaddrs(&ifaddr) == -1) {
        SEAICE_LOG_ERROR(logger) << "getifaddrs failed";
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL) {
            continue;
        }
        family = ifa->ifa_addr->sa_family;
        SEAICE_LOG_DEBUG(logger) << "name = " << ifa->ifa_name;
        if(family == AF_INET) {
            inet_ntop(AF_INET, &(((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr), 
                        ipstr, 16);
            inet_ntop(AF_INET, &(((struct sockaddr_in *)(ifa->ifa_netmask))->sin_addr), 
                        netmask, 16);
            SEAICE_LOG_DEBUG(logger) << "family ip4= " << family;
            SEAICE_LOG_DEBUG(logger) << ipstr << " : " << netmask;
        } else if (family == AF_INET6) {
            inet_ntop(AF_INET6, &(((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr), 
                        ipstr, 16);
            inet_ntop(AF_INET6, &(((struct sockaddr_in *)(ifa->ifa_netmask))->sin_addr), 
                        netmask, 16);
            SEAICE_LOG_DEBUG(logger) << "family ip6 = " << family;
            SEAICE_LOG_DEBUG(logger) << ipstr << " : " << netmask;
        }
    }
    freeifaddrs(ifaddr);
}

bool isLittleEndial() {
    union w {
        int a;
        char b;
    }w1;
    w1.a = 1;
    return (w1.b == 1);
}

//判断大小端
//小端低地址存放低位
void test_endian() {
    SEAICE_LOG_DEBUG(logger) << isLittleEndial();
}

template<int a, int b>
typename enable_if<a+b == 233, bool>::type is233() {
    return true;
}

template<int a, int b>
typename enable_if<a+b != 233, bool>::type is233() {
    return false;
}

//https://izualzhy.cn/SFINAE-and-enable_if
//测试 enable_if
//https://ouuan.github.io/post/c-11-enable-if-%E7%9A%84%E4%BD%BF%E7%94%A8/
void test_enable_if(){
    SEAICE_LOG_DEBUG(logger) << is233<1,232>() <<" , "<< is233<1,1>();
}

void test_iface() {
    std::multimap<std::string, std::pair<seaice::Address::ptr, uint32_t> >results;
    bool v = seaice::Address::GetInterfaceAddress(results);
    if(!v) {
        SEAICE_LOG_DEBUG(logger)<< "get interface failed";
        return;
    }
    for(auto& i : results) {
        SEAICE_LOG_DEBUG(logger) << i.first << " - " << i.second.first->toString() <<
            " - " << i.second.second;
    }
}

void test_ipv4() {
    auto addr = seaice::IPAddress::Create("127.0.0.1");
    if(addr) {
        SEAICE_LOG_DEBUG(logger) << addr->toString();
    }
}

void test() {
    std::vector<seaice::Address::ptr> addrs;
    bool v = seaice::Address::Lookup(addrs, "www.baidu.com");
    if(!v) {
        SEAICE_LOG_ERROR(logger) << "lookup failed";
    }
    for(size_t i = 0; i < addrs.size(); ++i) {
        SEAICE_LOG_DEBUG(logger) << i << " - " << addrs[i]->toString();
    }

    seaice::Address::ptr val = seaice::Address::LookupAny("www.baidu.com");
    seaice::IPAddress::ptr addr = std::dynamic_pointer_cast<seaice::IPAddress>(val);
    SEAICE_LOG_DEBUG(logger) << addr->toString();
    SEAICE_LOG_DEBUG(logger) << addr->networkAddress(15)->toString();
    //SEAICE_LOG_DEBUG(logger) << addr->subnetMask()->toString();
}

int main() {

    test_getaddrinfo();
    //test_getifaddrs();
    //test_endian();
    //test_enable_if();

    test_ipv4();
    test_iface();
    test();
    return 0;
}