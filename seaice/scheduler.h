#ifndef _SEAICE_SCHEDULER_H_
#define _SEAICE_SCHEDULER_H_

#include <memory>
#include <functional>
#include <list>
#include <vector>
#include <atomic>
#include <stdint.h>
#include <string>
#include "thread.h"
#include "fiber.h"
#include "mutex.h"
#include "utils.h"

namespace seaice {


class Scheduler {
public:
    typedef Mutex MutexType;
    typedef std::shared_ptr<Scheduler> ptr;

private:

    struct FiberAndThread {
        typedef std::function<void()> Callback;

        Fiber::ptr fiber = nullptr;
        std::function<void()> cb = nullptr;
        int threadId = -1;

        FiberAndThread(Fiber::ptr f, int tid) 
        : fiber(f), threadId(tid) {
        }

        FiberAndThread(Fiber::ptr* f, int tid) {
            fiber.swap(*f);
            threadId = tid;
        }

        FiberAndThread(Callback callback, int tid) 
        : cb(callback), threadId(tid){
        }

        FiberAndThread(Callback* callback, int tid) {
            cb.swap(*callback);
            threadId = tid;
        }

        FiberAndThread() 
        : threadId(-1){
        }

        void reset() {
            fiber = nullptr;
            cb = nullptr;
            threadId = -1;
        }

    };

public:
    Scheduler(int threadCount, const std::string name = "UNKNOW", bool use_caller = false);
    virtual ~Scheduler();

    void start();
    void stop();
    void mainFun();
    virtual void idleFun();
    virtual void tick();
    virtual bool isStoping();
    void setThis();

    static Scheduler* getThis();
    const std::string& getName() const { return m_name;}

    template<typename FiberOrCb>
    bool schedule(FiberOrCb fc, int threadId = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, threadId);
        }
        return need_tickle;
    }

    template<typename FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int threadId = 1) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, threadId);
        //std::cout << "sheduleNolock" << std::endl;
        //std::cout << seaice::utils::print_backtrace() << std::endl;
        m_fibers.push_back(ft);
        return need_tickle;
    }

private:
    int m_thread_count;
    std::string m_name;
    Fiber::ptr m_root_fiber = nullptr;
    int m_root_threadId = 0;
    bool m_stop = true;
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::vector<uint32_t> m_threadIds;
    std::list<FiberAndThread> m_fibers;
    std::atomic<uint64_t> m_active_thread;
    std::atomic<uint64_t> m_idle_thread;
};



}

#endif