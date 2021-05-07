#include "mutex.h"
#include "scheduler.h"
#include "macro.h"

using namespace seaice;

Semaphore::Semaphore(uint32_t value) {
    sem_init(&m_sem, 0, value);
}

Semaphore::~Semaphore() {
    sem_destroy(&m_sem);
}

void Semaphore::wait() {
    sem_wait(&m_sem);
}

void Semaphore::notify() {
    sem_post(&m_sem);
}

FiberSemaphore::FiberSemaphore(size_t num)
    : m_num(num) {
}

bool FiberSemaphore::tryWait() {
    SEAICE_ASSERT(seaice::Scheduler::getThis());
    {
        MutexType::Lock lock(m_mutex);
        if(m_num > 0u) {
            --m_num;
            return true;
        }
        return false;
    }
}

void FiberSemaphore::wait() {
    SEAICE_ASSERT(seaice::Scheduler::getThis());
    {
        MutexType::Lock lock(m_mutex);
        if(m_num > 0u) {//可以同时处理 m_num 个，
            --m_num;
            return;
        }
        m_list.push_back(std::make_pair(Scheduler::getThis(), Fiber::getThis()));
    }
    Fiber::yieldToHold();
}

void FiberSemaphore::notify() {
    MutexType::Lock lock(m_mutex);
    if(!m_list.empty()) {
        auto next = m_list.front();
        m_list.pop_front();
        next.first->schedule(next.second);
    } else {
        ++m_num;
    }
}