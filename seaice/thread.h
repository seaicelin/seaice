#ifndef _SEAICE_THREAD_H_
#define _SEAICE_THREAD_H_

#include <stdint.h>
#include <functional>
#include <memory>
#include "mutex.h"
#include "noncopyable.h"

namespace seaice {

class Thread : Noncopyable {
public:
    typedef std::shared_ptr<Thread> ptr;
    typedef std::function<void()> Callback;

    Thread(Callback cb, const std::string name = "UNKNOW");
    ~Thread();

    static void* run(void* arg);

    void join();
    void detach();

    //const pthread_t getTid() const { return m_tid; }
    static const std::string& GetName();
    static void SetName(const std::string& name);
    static Thread* GetThis();
    static const pid_t GetThreadId();
private:
    Callback m_cb;
    std::string m_name;
    pthread_t m_thread;
    pid_t m_id = -1;
    Semaphore m_semaphore;
};


}

#endif