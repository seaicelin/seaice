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
//#include "timer.h"

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

int main(int argc, char** args)
{
    Thread::SetName("main");
    SEAICE_LOG_DEBUG(logger) << "main start";



    IOManager iom(2);
    iom.start();

    Timer::ptr timer = Timer::ptr(new Timer(3000,
        [](){
            SEAICE_LOG_DEBUG(logger) << "timer call back ======== 3s";
        }, true));

    Timer::ptr timer2 = Timer::ptr(new Timer(2000,
        [](){
            SEAICE_LOG_DEBUG(logger) << "timer call back ========= 2s";
        }, true));

    iom.addTimer(timer);
    iom.addTimer(timer2);
    timer->cancel();
    //iom.schedule(fiber2);
    iom.stop();


    SEAICE_LOG_DEBUG(logger) << "main end";

    return 0;
}