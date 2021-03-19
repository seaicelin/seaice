#include <iostream>
#include "yaml-cpp/yaml.h"
#include "../seaice/log.h"
#include "../seaice/fiber.h"
#include "../seaice/scheduler.h"
#include <unistd.h>
#include <ucontext.h>

using namespace seaice;
using namespace std;

static Logger::ptr logger = SEAICE_LOGGER("system");

void foo(void) {
    SEAICE_LOG_DEBUG(logger) << "foo start";
    Fiber::yieldToReady();
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

int main(int argc, char** args)
{
    Thread::SetName("main");
    Scheduler sch(1);
    sch.start();
    SEAICE_LOG_DEBUG(logger) << "main start";
    
    Fiber::ptr fiber2 = Fiber::ptr(new Fiber(std::bind(&foo)));

    //std::function<void()> callback = std::bind(cb);
    for(int i = 0; i < 10; i++){
        sch.schedule(&cb);
        Fiber::ptr fiber = Fiber::ptr(new Fiber(std::bind(&bar)));
        sch.schedule(fiber, -1);
        Fiber::ptr fiber2 = Fiber::ptr(new Fiber(std::bind(&foo)));
        sch.schedule(fiber2, -1);
    }
    sch.stop();
    SEAICE_LOG_DEBUG(logger) << "main end";

    return 0;
}