#include <iostream>
#include "yaml-cpp/yaml.h"
#include "../seaice/log.h"
#include "../seaice/fiber.h"
#include "../seaice/iomanager.h"
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
    hint.ai_socktype = SOCK_STREAM;

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

    if(getifaddrs(&ifaddr) == -1) {
        SEAICE_LOG_ERROR(logger) << "getifaddrs failed";
    }

    for(ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
        if(ifa->ifa_addr == NULL) {
            continue;
        }
        family = ifa->ifa_addr->sa_family;
        SEAICE_LOG_DEBUG(logger) << "name = " << ifa->ifa_name <<
                " family = " << family;
        if(family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                (family == AF_INET)? sizeof(struct sockaddr_in) :
                                     sizeof(struct sockaddr_in6),
                host, NI_MAXHOST,
                NULL, 0, NI_NUMERICHOST);
            if(s!=0) {
                SEAICE_LOG_ERROR(logger) << "getnameinfo failed";
            }
        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
                   struct rtnl_link_stats *stats = (rtnl_link_stats *)ifa->ifa_data;

                   printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                          "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                          stats->tx_packets, stats->rx_packets,
                          stats->tx_bytes, stats->rx_bytes);
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

int main() {

    test_getaddrinfo();
    test_getifaddrs();
    test_endian();
    test_enable_if();
    return 0;
}