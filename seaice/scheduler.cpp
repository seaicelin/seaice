#include "hook.h"
#include "log.h"
#include "macro.h"
#include "scheduler.h"
#include "utils.h"

namespace seaice {

static Scheduler* t_scheduler = nullptr;
static thread_local Fiber::ptr t_scheduler_fiber = nullptr;

static Logger::ptr logger = SEAICE_LOGGER("system");

Scheduler::Scheduler(int threadCount, const std::string name, bool use_caller)
    : m_thread_count(threadCount)
    , m_name(name) {/*
    if(use_caller == true) {
        m_root_threadId = utils::getThreadId();
        m_root_fiber = Fiber::ptr(new Fiber(std::bind(&Scheduler::mainFun, this)));
        m_threadIds.push_back(m_root_threadId);
        t_scheduler_fiber = m_root_fiber;
    } else {*/
        m_root_threadId = -1;
    //}
}

Scheduler::~Scheduler() {
}

void Scheduler::start() {
    SEAICE_LOG_DEBUG(logger) << "start scheduler thread";
    setThis();
    m_stop = false;

    for (int i = 0; i < m_thread_count; ++i) {
        Thread::ptr thread = Thread::ptr(new Thread(std::bind(&Scheduler::mainFun, this), 
                                            "thread_" + std::to_string(i)));
        m_threads.push_back(thread);
        m_threadIds.push_back(thread->GetThreadId());
    }

}

void Scheduler::stop() {
    m_stop = true;
    std::vector<Thread::ptr> threads;
    threads.swap(m_threads);
    for(auto thread : threads) {
        thread->join();
    }
}

bool Scheduler::isStoping() {
    MutexType::Lock lock(m_mutex);
    return m_stop && m_fibers.empty();
}

void Scheduler::setThis() {
    t_scheduler = this;
}

Scheduler* Scheduler::getThis() {
    return t_scheduler;
}

void Scheduler::mainFun() {
    SEAICE_LOG_DEBUG(logger) << "scheduler mainFun start";
    seaice::set_hook_enable(true);
    Fiber::ptr mainFiber = Fiber::getMainFiber();
    Fiber::ptr idleFiber = Fiber::ptr(new Fiber(std::bind(&Scheduler::idleFun, this)));
    Fiber::ptr cb_fiber = nullptr;
    FiberAndThread ft;
    bool need_tick = false;
    while(true) {
        SEAICE_LOG_DEBUG(logger) << "scheduler mainFun middle";
        ft.reset();
        {
            MutexType::Lock lock(m_mutex);
            //SEAICE_LOG_DEBUG(logger) << "start ticle" << 
                    //" fiber size = " << m_fibers.size();
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                if(it->threadId != -1 && it->threadId != utils::getThreadId()) {
                    ++it;
                    continue;
                }
                SEAICE_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }
                ft = (*it);
                m_fibers.erase(it);
                //SEAICE_LOG_DEBUG(logger) << "end ticle" << 
                    //" fiber size = " << m_fibers.size();
                break;
            }
            need_tick = (!m_fibers.empty());
        }
        if(ft.fiber) {
            Fiber::State state = ft.fiber->getState();
            //SEAICE_LOG_DEBUG(logger) << "fiber swap in" << ft.fiber->toString();
            //SEAICE_ASSERT(state != Fiber::TERM);
            SEAICE_ASSERT(state != Fiber::EXECPT);
            SEAICE_ASSERT(state != Fiber::EXEC);
            //if(state == Fiber::INIT || state == Fiber::READY) {
            if(state != Fiber::TERM && state != Fiber::EXECPT) {
                ft.fiber->swapIn();
            }
            //}
            state = ft.fiber->getState();
            //SEAICE_LOG_DEBUG(logger) << "fiber swap out" << ft.fiber->toString();
            /*if(state == Fiber::HOLD || state == Fiber::READY) {
                //need_tick = schedule(ft.fiber, ft.threadId);
            } else */
            /*if(state != Fiber::TERM) {
                SEAICE_LOG_ERROR(logger) << "fiber error state = " << state
                    << " fiber id = " << ft.fiber->getId();
            }*/
        } else if(ft.cb) {
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
               cb_fiber = Fiber::ptr(new Fiber(ft.cb));
            }
            Fiber::State state = cb_fiber->getState();
            //SEAICE_LOG_DEBUG(logger) << "cb swap in" << cb_fiber->toString();
            SEAICE_ASSERT(state != Fiber::TERM);
            SEAICE_ASSERT(state != Fiber::EXECPT);
            SEAICE_ASSERT(state != Fiber::EXEC);
            cb_fiber->swapIn();
            //SEAICE_LOG_DEBUG(logger) << "cb swap out" << cb_fiber->toString();
            state = cb_fiber->getState();
            if(state == Fiber::HOLD || state == Fiber::READY) {
                //need_tick = schedule(cb_fiber, ft.threadId);
            } else if(state != Fiber::TERM) {
                SEAICE_LOG_ERROR(logger) << "cb error state = " << state
                    << " fiber id = " << cb_fiber->getId();
            }
            if(state != Fiber::TERM) {
                cb_fiber.reset();
            }
        }

        //fiber队列不是空，需要继续循环执行
        if(need_tick) {
            need_tick = false;
            tick();
            continue;
        }

        if(idleFiber) {
            idleFiber->swapIn();
            if(idleFiber->getState() == Fiber::TERM ||
                idleFiber->getState() == Fiber::EXECPT) {
                break;
            }
        }
    }
    SEAICE_LOG_DEBUG(logger) << "scheduler mainFun end";
}

void Scheduler::idleFun() {
    while(!isStoping()) {
        Fiber::yieldToHold();
    }
}

void Scheduler::tick() {
    SEAICE_LOG_DEBUG(logger) << " tick ";
}

}