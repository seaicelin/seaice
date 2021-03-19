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
#include <dlfcn.h>
#include <stdlib.h>
#include "../seaice/timer.h"
#include <unistd.h>
#include "../seaice/hook.h"

using namespace seaice;
using namespace std;

static Logger::ptr logger = SEAICE_LOGGER("system");

/*
这篇文章写的不错，基于云风协程库的协程原理解读
https://blog.csdn.net/u011228889/article/details/79759834
https://zhengyinyong.com/post/ucontext-usage-and-coroutine/
*/

static int testCount = 0;

//dlsym 需要添加编译参数
//-Wl,--no-as-needed -ldl

//read_func read_f = (read_fun)dlsym(RTLD_NEXT, "read");
sleep_fun sleep_ff = (sleep_fun)dlsym(RTLD_NEXT, "sleep");
/*
extern "C" {
void* malloc(size_t size) {
    //void* (*my_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
    void* (*my_malloc)(size_t) = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
    testCount++;
    SEAICE_LOG_DEBUG(logger) << "testCount = " << testCount;
    return my_malloc(size);
}

ssize_t read(int fd, void *buf, size_t count) {
    if(!seaice::is_hook_enable()) {
        return read_f(fd, buf, count);
    }
}

}
*/

void test_sleep() {
    seaice::IOManager iom(2);
    iom.start();

    iom.schedule([](){
        SEAICE_LOG_DEBUG(logger) << "1";
        sleep(3);
        SEAICE_LOG_DEBUG(logger) << "3";
    });

    iom.schedule([](){
        SEAICE_LOG_DEBUG(logger) << "4";
        sleep(3);
        SEAICE_LOG_DEBUG(logger) << "5";
    });

    SEAICE_LOG_DEBUG(logger) << "2";
    iom.stop();
}

int main(int argc, char** args)
{
    //seaice::_HookIniter s_hook_initer;
    Thread::SetName("main");
    SEAICE_LOG_DEBUG(logger) << "main start";

    //sleep_ff(3);
    test_sleep();
/*
    int* p1 = (int*)malloc(sizeof(int));
    int* p2 = (int*)malloc(sizeof(int));
    int* p3 = (int*)malloc(sizeof(int));
    int* p4 = (int*)malloc(sizeof(int));

    free(p1);
    free(p2);
    free(p3);
    free(p4);
*/

    SEAICE_LOG_DEBUG(logger) << "main end";

    return 0;
}