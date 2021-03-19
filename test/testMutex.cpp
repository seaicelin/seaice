#include <iostream>
#include "../seaice/log.h"
#include "yaml-cpp/yaml.h"
#include "../seaice/mutex.h"
#include "../seaice/thread.h"
#include "../seaice/macro.h"
#include <vector>
#include <string>
#include <execinfo.h>


class B {
public:
    B() {
        std::cout<<"B()" << std::endl;
    }

    B(const B& b) {
        std::cout<<"B(const B& b)" << std::endl;
    }

    B(B& b) {
        std::cout<<"B(B& b)" << std::endl;
    }
    int a = 5;
};

class A {
public:
    A(B v)
    : a(v) {
    }
    ~A(){}
private:
    B& a;
};

long long sum = 0;

static seaice::Logger::ptr logger = SEAICE_LOGGER("seaice");

typedef seaice::SpinLock MutexType;
//typedef seaice::Mutex::Lock MutexType;
//typedef seaice::RWMutex MutexType;
void printStack();
MutexType mutex;

void callback() {
    {
        MutexType::Lock lock(mutex);
        for(int i = 0; i < 10; i++) {
            sum++;
        }
    }

    //while(1);
    //SEAICE_LOG_DEBUG(logger) << "thread_name = " << seaice::Thread::GetName()
        //<< " thread_id = " << seaice::Thread::GetThreadId() << std::endl;
}


int main(int argc, char** argv)
{
    //测试定义引用成员变量
    //测试拷贝构造函数
    B b;
    A t = A(b);

    using namespace seaice;


    std::vector<Thread::ptr> threadList;

    //linux 线程可以创建的最大数目 cat /proc/sys/kernel/pid_max
    for(int i = 0; i < 2; i++) {
        Thread::ptr thread = Thread::ptr(new Thread(std::bind(&callback), "thread_" + std::to_string(i)));
        threadList.push_back(thread);
    }

    for(auto thread: threadList) {
        thread->join();
    }

    SEAICE_LOG_DEBUG(logger) << "sum = " << sum;

    //SEAICE_ASSERT(true);
    //SEAICE_ASSERT2(false, "assert here2");


    return 0;
}