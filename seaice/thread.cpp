#include "thread.h"
#include "log.h"
#include "utils.h"
#include <thread>

namespace seaice {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";
static thread_local pid_t t_pid = -1;

static Logger::ptr logger = SEAICE_LOGGER("system");

const std::string& Thread::GetName() { 
    return t_thread_name;
}

void Thread::SetName(const std::string& name) {
    if(name.empty()) {
        return;
    }
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread* Thread::GetThis() {
    return t_thread;
}

const pid_t Thread::GetThreadId() {
    return t_pid;
}

Thread::Thread(Callback cb, const std::string name) 
    : m_cb(cb)
    , m_name(name) {
    int ret = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(ret) {
        SEAICE_LOG_ERROR(logger) << "pthread_create failed, ret = " << ret
            << " name = " << m_name;
        throw std::logic_error("pthread_create error");
    }
    //SEAICE_LOG_DEBUG(logger) << "thread contructor wait";
    m_semaphore.wait();
}

Thread::~Thread() {
    //detach();
}

void Thread::join() {
    if(m_thread) {
        int ret = pthread_join(m_thread, nullptr);
        if(ret) {
            SEAICE_LOG_ERROR(logger) << "pthread_join failed, ret = " << ret
                << " name = " << m_name;
            throw std::logic_error("pthread_join error");
        }
    }
}

void* Thread::run(void* arg) {
    Thread* self = (Thread*)arg;
    t_thread = self;
    t_thread_name = self->m_name;
    t_pid = utils::getThreadId();
    self->m_id = t_pid;
    pthread_setname_np(pthread_self(), self->m_name.substr(0, 15).c_str());

    Callback cb;
    cb.swap(self->m_cb);
    //cb = self->m_cb;
    //SEAICE_LOG_DEBUG(logger) << "thread run notify";
    self->m_semaphore.notify();
    cb();
    //self->m_cb();
    //SEAICE_LOG_DEBUG(logger) << "thread run cb";
    return 0;
}

void Thread::detach() {
    if(m_thread) {
        int ret = pthread_detach(m_thread);
        if(ret) {
            SEAICE_LOG_ERROR(logger) << "pthread_detach failed, ret = " << ret
                << " name = " << m_name;
            throw std::logic_error("pthread_detach error");
        }
    }
}

}