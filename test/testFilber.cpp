#include <iostream>
#include "yaml-cpp/yaml.h"
#include "../seaice/log.h"
#include "../seaice/fiber.h"
#include <unistd.h>
#include <ucontext.h>

using namespace seaice;
using namespace std;

static Logger::ptr logger = SEAICE_LOGGER("system");

ucontext_t context1, context2, context_main;

void foo(void) {
    SEAICE_LOG_DEBUG(logger) << "foo start";
    Fiber::yieldToHold();
    SEAICE_LOG_DEBUG(logger) << "foo end";
    //printf("foo\n");
}

void bar() {
    SEAICE_LOG_DEBUG(logger) << "bar start";
    Fiber::yieldToHold();//(&context2, &context1);
    SEAICE_LOG_DEBUG(logger) << "bar end";
    //Fiber::yieldToHold();
    //printf("bar\n");
}
/*
这篇文章写的不错，基于云风协程库的协程原理解读
https://blog.csdn.net/u011228889/article/details/79759834
https://zhengyinyong.com/post/ucontext-usage-and-coroutine/
*/

int main(int argc, char** args)
{
    Fiber::getMainFiber();
    SEAICE_LOG_DEBUG(logger) << "main start";
    Fiber::ptr fiber = Fiber::ptr(new Fiber(std::bind(&bar)));
    fiber->swapIn();

    Fiber::ptr fiber2 = Fiber::ptr(new Fiber(std::bind(&foo)));
    fiber2->swapIn();

    fiber->swapIn();
    fiber2->swapIn();
    //fiber->swap_in();
    //Fiber::yieldToResume();

    SEAICE_LOG_DEBUG(logger) << "main end";
    /*
    char stack1[102400];
    char stack2[102400];
       
    getcontext(&context1);
    context1.uc_stack.ss_sp = stack1;
    context1.uc_stack.ss_size = sizeof(stack1);
    context1.uc_link = &context_main;
    makecontext(&context1, foo, 0);
       
    getcontext(&context2);
    context2.uc_stack.ss_sp = stack2;
    context2.uc_stack.ss_size = sizeof(stack2);
    context2.uc_link = &context1;
    makecontext(&context2, bar, 0);

    printf("Hello world\n");
    sleep(1);
    swapcontext(&context_main, &context2);
    //setcontext(&context2);*/
    return 0;
}