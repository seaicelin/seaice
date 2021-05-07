#ifndef _SEAICE_MUTEX_H_
#define _SEAICE_MUTEX_H_

#include <semaphore.h>
#include <pthread.h>
#include <iostream>
#include <list>
#include <memory>
#include <stdint.h>
#include "fiber.h"
#include "noncopyable.h"

namespace seaice {

class Semaphore : public seaice::Noncopyable {

public:
    Semaphore(uint32_t value = 1);
    ~Semaphore();

    void wait();
    void notify();

private:
    sem_t m_sem;
};

template<typename T>
class ScopedLockImpl {
public:
    ScopedLockImpl(T& m) 
    : m_mutex(m) {
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl() {
        m_mutex.unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

template<typename T>
class ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& m)
    : m_mutex(m) {
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl() {
        m_mutex.unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

template<typename T>
class WriteScopedLockImpl{
public:
    WriteScopedLockImpl(T& m)
    : m_mutex(m) {
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl() {
        m_mutex.unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    //在类中定义引用变量, 必须要在初始化列表中初始化该成员变量
    //不能有默认构造函数，必须提供构造函数
    //构造函数的形参必须为引用类型
    T& m_mutex;
    bool m_locked;
};

class Mutex : public seaice::Noncopyable {
public:
    typedef ScopedLockImpl<Mutex> Lock;

    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }

private:
    pthread_mutex_t m_mutex;
};

class SpinLock : public seaice::Noncopyable {
public:
    typedef ScopedLockImpl<SpinLock> Lock;

    SpinLock() {
        pthread_spin_init(&m_spinLock, 0);

    }
    ~SpinLock() {
        pthread_spin_destroy(&m_spinLock);
    }

    void lock() {
        pthread_spin_lock(&m_spinLock);
    }

    void unlock() {
        pthread_spin_unlock(&m_spinLock);
    }

private:
    pthread_spinlock_t m_spinLock;
};



class RWMutex : public seaice::Noncopyable{
public:
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    typedef ReadScopedLockImpl<RWMutex> ReadLock;

    RWMutex() {
        pthread_rwlock_init(&m_rwLock, NULL);
    }
    ~RWMutex() {
        pthread_rwlock_destroy(&m_rwLock);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_rwLock);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_rwLock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_rwLock);
    }
private:
    pthread_rwlock_t m_rwLock;
};

class NullWRWmutex : public seaice::Noncopyable {
public:
    typedef ReadScopedLockImpl<NullWRWmutex> ReadLock;
    typedef WriteScopedLockImpl<NullWRWmutex> WriteLock;

    NullWRWmutex() {}
    ~NullWRWmutex() {}

    void rdlock();
    void wrlock();
    void unlock();
};

class NullMutex : public seaice::Noncopyable {
public:
    typedef ScopedLockImpl<NullMutex> Lock;

    NullMutex() {}
    ~NullMutex() {}

    void lock() {}
    void unlock() {}

};

class Scheduler;
//可以同时处理N个 fiber, 超过则放入等待队列
class FiberSemaphore : public seaice::Noncopyable {
public:
    typedef SpinLock MutexType;
    FiberSemaphore(size_t num);

    bool tryWait();
    void wait();
    void notify();
private:
    size_t m_num;
    std::list<std::pair<Scheduler*, Fiber::ptr> > m_list;
    MutexType m_mutex;
};

}

#endif