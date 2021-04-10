#include "timer.h"
#include "utils.h"
#include "macro.h"
#include <sys/timerfd.h>
#include <list>
#include "log.h"

namespace seaice {


static Logger::ptr logger = SEAICE_LOGGER("system");

static std::atomic<uint64_t> s_timer_id{0};
//不使用 timerfd
//iomanager 使用一个线程来监听 timerfd 的话，可能当前线程
//在处理其他任务，感觉可能会导致精度降低
//而使用epoll的timerout 机制，则每个线程都可以在超时的时候
//返回

Timer::Timer(uint64_t time, Callback cb, bool cycle)
    : m_ms(time)
    , m_cb(cb)
    , m_cycle(cycle)
    , m_id(s_timer_id++)
    , m_cancel(false) {
    m_expire_ms = m_ms + utils::getCurrentMs();
    SEAICE_LOG_DEBUG(logger) << "timer id = " << m_id;
    //SEAICE_ASSERT(false);
}

Timer::~Timer() {
    SEAICE_LOG_DEBUG(logger) << "~timer id = " << m_id;
}

void Timer::cancel() {
    m_cancel = true;
    m_cb = nullptr;
    m_cycle = false;
}

void Timer::refresh() {
    if(m_cycle == true) {
        m_expire_ms = (m_ms + utils::getCurrentMs());
    }
}

uint64_t Timer::getExpireTime() {
    return m_expire_ms;
}

void Timer::setExpireTime(uint64_t time) {
    m_expire_ms = time;
}

bool TimerCmp::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) {
    if(!lhs && !rhs) {
        return false;
    }
    if(!lhs) {
        return true;
    }
    if(!rhs) {
        return false;
    }
    if(lhs->m_expire_ms < rhs->m_expire_ms) {
        return true;
    }
    if(rhs->m_expire_ms < lhs->m_expire_ms) {
        return false;
    }
    return lhs.get() < rhs.get();
}


TimerManager::TimerManager() {
    m_timer = Timer::ptr(new Timer(0, nullptr));
    SEAICE_LOG_DEBUG(logger) << "TimerManager create ";
}

TimerManager::~TimerManager() {
}

void TimerManager::addTimer(Timer::ptr timer) {
    RWMutexType::WriteLock lock(m_mutex);
    if(!timer->m_cancel) {
        bool need_tick = m_timerSet.empty();
        m_timerSet.insert(timer);
        if(need_tick) {
            insertTimerAtFrontCallback();
        }
    }
}

Timer::ptr TimerManager::addTimer(uint64_t millisecond, std::function<void()> cb, 
        bool cycle) {
    Timer::ptr timer = Timer::ptr(new Timer(millisecond, cb, cycle));
    addTimer(timer);
    return timer;
}

void TimerManager::delTimer(Timer::ptr timer) {
    RWMutexType::WriteLock lock(m_mutex);
    timer->cancel();
    m_timerSet.erase(timer);
}

uint64_t TimerManager::getRecentTimerout() {
    int ret = INT32_MAX;
    RWMutexType::WriteLock lock(m_mutex);
    if(!m_timerSet.empty()) {
        auto it = m_timerSet.begin();
        ret = (*it)->getExpireTime() - getCurrentMillisecond();
    }
    if(ret < 0) {
        ret = 0;
    }
    return ret;
}

uint64_t TimerManager::getCurrentMillisecond() {
    return utils::getCurrentMs();
}

void TimerManager::listTimeroutCallback(std::list<Callback>& callbacks) {
    m_timer->setExpireTime(getCurrentMillisecond());
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timerSet.empty()) {
            return;
        }
    }
    std::vector<Timer::ptr> vec;
    {
        RWMutexType::WriteLock lock(m_mutex);
        auto end = m_timerSet.lower_bound(m_timer);
        auto begin = m_timerSet.begin();
        //SEAICE_LOG_DEBUG(logger) << "start timer set size = " << m_timerSet.size();
        vec.assign(begin, end);
        //SEAICE_LOG_DEBUG(logger) << "list callback size = " << vec.size();
        m_timerSet.erase(begin, end);
        //SEAICE_LOG_DEBUG(logger) << "end timer set size = " << m_timerSet.size();
    }
    callbacks.clear();
    for(auto it : vec) {
        if(!it->m_cancel) {
            callbacks.push_back(it->m_cb);
            SEAICE_LOG_DEBUG(logger) << /*"it expire = " << it->getExpireTime()
                    << " cur expire = " << m_timer->getExpireTime()
                    <<*/ " diff = " << m_timer->getExpireTime() - it->getExpireTime()
                    << " timer id = " << it->m_id << " cycle = " << it->m_cycle;
            if(it->m_cycle) {
                it->refresh();
                addTimer(it);
            }
        }
    }
}

void TimerManager::insertTimerAtFrontCallback() {
}

}