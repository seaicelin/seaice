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

using namespace seaice;
using namespace std;

static Logger::ptr logger = SEAICE_LOGGER("system");

void foo(void) {
    SEAICE_LOG_DEBUG(logger) << "foo start";
    //Fiber::yieldToReady();
    SEAICE_LOG_DEBUG(logger) << "foo end";
}

void bar() {
    SEAICE_LOG_DEBUG(logger) << "bar start";
    //Fiber::yieldToHold();
    SEAICE_LOG_DEBUG(logger) << "bar end";
}

void cb() {
    SEAICE_LOG_DEBUG(logger) << "cb start";
    //Fiber::yieldToHold();
    SEAICE_LOG_DEBUG(logger) << "cb end";
}
/*
这篇文章写的不错，基于云风协程库的协程原理解读
https://blog.csdn.net/u011228889/article/details/79759834
https://zhengyinyong.com/post/ucontext-usage-and-coroutine/
*/

int createSoket() {
    int sock = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    return sock;
}

void test_connect() {
    const char* ip = "127.0.1.1";
    int port = 9988;
    int sock = createSoket();
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int rt = connect_with_timeout(sock, (sockaddr*)&address, sizeof(address), 3000);
    SEAICE_LOG_DEBUG(logger) << "connect socket rt = " << rt;

    timeval t;
    t.tv_sec = 5;
    t.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t));
    auto iom = (IOManager*)IOManager::getThis();
    iom->addEvent(sock, IOManager::WRITE, [sock](){
        SEAICE_LOG_DEBUG(logger) << "sock write call abck";
        std::string str("hello seaice");

        LogIn login;
        strcpy(login.username, "seaicelin");
        strcpy(login.password, "12344566d");
        //return pClient->sendData((const char*)&login, login.hdr.len);
        int n = send(sock, (const char*)&login, login.hdr.len, 0);
        SEAICE_LOG_DEBUG(logger) << "n = " << n;
    });
    iom->addEvent(sock, IOManager::READ, [sock](){
        SEAICE_LOG_DEBUG(logger) << "sock read call abck";
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        int n = recv(sock, buf, sizeof(buf), 0);
        SEAICE_LOG_DEBUG(logger) << "n = " << n << " buf = " <<
                buf;
    });
    //Fiber::yieldToHold();
}

int main(int argc, char** args)
{
    //Scheduler::ptr sch = Scheduler::ptr(new Scheduler(1));
    //sch->start();
    Thread::SetName("main");
    //Scheduler sch(2);
    //sch.start();
    SEAICE_LOG_DEBUG(logger) << "main start";
    seaice::set_hook_enable(true);
    //SIGPIPE 13  涉及管道和socket
    //在TCP连接半关闭状态下，其退化成了单工通信，即一方只能发，一方只能收。
    //此时，如果让只能收的一方去调用write函数发数据，会导致TCP给其回复一个RST段，
    //之后如果再调用write就会产生SIGPIPE信号，对于这个信号的处理我们通常忽略即可。
    //signal（SIGPIPE，SIG_IGN）
    //默认的SIGPIPE信号的处理方式为exit当前进程。
    signal(SIGPIPE, SIG_IGN);

    IOManager iom(2);
    iom.start();
    iom.schedule(&test_connect);

    /*
    Fiber::ptr fiber2 = Fiber::ptr(new Fiber(std::bind(&foo)));
    iom.schedule(fiber2);

    Timer::ptr timer = Timer::ptr(new Timer(3000,
        [](){
            SEAICE_LOG_DEBUG(logger) << "timer call back ======== 3s";
        }, true));

    Timer::ptr timer2 = Timer::ptr(new Timer(2000,
        [](){
            SEAICE_LOG_DEBUG(logger) << "timer call back ========= 2s";
        }, true));

    iom.addTimer(timer);
    iom.addTimer(timer2);*/

    iom.stop();

    //std::function<void()> callback = std::bind(cb);
    //for(int i = 0; i < 10; i++){
        //sch.schedule(&cb);
        //Fiber::ptr fiber = Fiber::ptr(new Fiber(std::bind(&bar)));
        //sch.schedule(fiber, -1);
        //Fiber::ptr fiber2 = Fiber::ptr(new Fiber(std::bind(&foo)));
        //sch.schedule(fiber2, -1);
    //}
    //sch.stop();
    SEAICE_LOG_DEBUG(logger) << "main end";

    return 0;
}