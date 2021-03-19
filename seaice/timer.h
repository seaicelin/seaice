#ifndef _SEAICE_TIMER_H_
#define _SEAICE_TIMER_H_

#include <stdint.h>
#include <memory>
#include <set>
#include <functional>
#include <list>
#include <atomic>
#include "mutex.h"

namespace seaice{

class TimerManager;

class Timer {

friend class TimerManager;
friend class TimerCmp;

public:
    typedef std::shared_ptr<Timer> ptr;
    typedef std::function<void()> Callback;
    typedef RWMutex RWMutexType;

    Timer(uint64_t time, Callback cb, bool cycle = false);
    ~Timer();

    void cancel();

private:
    void refresh();
    uint64_t getExpireTime();
    void setExpireTime(uint64_t time);
    bool isDelete();
private:
    uint64_t m_ms;
    Callback m_cb;
    bool m_cycle;
    uint64_t m_id;
    std::atomic<bool> m_cancel;
    uint64_t m_expire_ms;
};

class TimerCmp{
public:
    bool operator()(const Timer::ptr& a, const Timer::ptr& b);
};

class TimerManager {
public:
    typedef std::shared_ptr<TimerManager> ptr;
    typedef RWMutex RWMutexType;
    typedef std::function<void()> Callback;

    TimerManager();
    ~TimerManager();

    void addTimer(Timer::ptr timer);
    Timer::ptr addTimer(uint64_t millisecond, std::function<void()> cb, 
        bool cycle = false);
    void delTimer(Timer::ptr timer);
    uint64_t getRecentTimerout();
    uint64_t getCurrentMillisecond();
    void listTimeroutCallback(std::list<Callback>& callbacks);
    virtual void insertTimerAtFrontCallback();

private:
    //int m_timerFd;
    std::set<Timer::ptr, TimerCmp> m_timerSet;
    RWMutexType m_mutex;
    Timer::ptr m_timer;
};

}

#endif