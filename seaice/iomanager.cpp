#include "iomanager.h"
#include <unistd.h>
#include "macro.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <algorithm>
#include "log.h"
#include "stdint.h"

namespace seaice {

static Logger::ptr logger = SEAICE_LOGGER("system");

void IOManager::FdContext::triggerEvent(int event) {
    //SEAICE_LOG_DEBUG(logger) << "triggerEvent event = " << event;
    events &= ~event;
    if(event & READ) {
        SEAICE_ASSERT(read.sch);
        if(read.cb) {
            read.sch->schedule(read.cb);
        } else if(read.fiber) {
            read.sch->schedule(read.fiber);
        } else {
            SEAICE_LOG_DEBUG(logger) << "trigger read event no cb or fiber";
        }
    }
    if(event & WRITE) {
        SEAICE_ASSERT(write.sch);
        if(write.cb) {
            write.sch->schedule(write.cb);
        } else if(write.fiber) {
            write.sch->schedule(write.fiber);
        } else {
            SEAICE_LOG_DEBUG(logger) << "trigger write event no cb or fiber";
        }
    }
}

IOManager::IOManager(int threadCount, const std::string name, bool use_caller)
    : Scheduler(threadCount, name, use_caller) {
    SEAICE_LOG_DEBUG(logger) << "IOManager contructor";

    m_epollfds = epoll_create(5000);
    if(m_epollfds == -1) {
        SEAICE_LOG_ERROR(logger) << "m_epollfds = " << m_epollfds
            << " errno = " << strerror(errno);
        SEAICE_ASSERT(false);
    }

    int rt = pipe(m_tickFds);
    if(rt == -1) {
        SEAICE_LOG_ERROR(logger) << "rt = " << rt
            << " errno = " << strerror(errno);
        SEAICE_ASSERT(false);
    }

    rt = fcntl(m_epollfds, F_SETFL, O_NONBLOCK);
    SEAICE_ASSERT(rt == 0);

    epoll_event epevent;
    memset(&epevent, 0, sizeof(epoll_event));
    epevent.events = EPOLLIN;
    epevent.data.fd = m_tickFds[0];
    rt = epoll_ctl(m_epollfds, EPOLL_CTL_ADD, m_tickFds[1], &epevent);
    if(rt != 0) {
        SEAICE_LOG_ERROR(logger) << "rt = " << rt
            << " errno = " << strerror(errno);
        SEAICE_ASSERT(false);
    }

    resizeFdContext(64);

    //start();
}

IOManager::~IOManager() {
    std::vector<FdContext>::size_type i;
    for(i = 0; i < mFdCtxs.size(); i++) {
        delete mFdCtxs[i];
    }
    close(m_tickFds[0]);
    close(m_tickFds[1]);
    close(m_epollfds);
}

int IOManager::addEvent(int fd, int event, std::function<void()> cb) {
    if(event == NONE || fd < 0) {
        SEAICE_LOG_ERROR(logger) <<"addEvent fd invalid " <<
            "or event is NONE";
        return -1;
    }

    resizeFdContext(fd);

    RWMutexType::ReadLock lock(m_mutex);
    FdContext* ptr = mFdCtxs[fd];
    lock.unlock();

    SEAICE_ASSERT(ptr);

    MutexType::Lock lock2(ptr->mutex);
    int curEvents = ptr->events;
    if(event & READ) {
        //ptr->read.cb.swap(cb);
        ptr->read.fiber = Fiber::getThis();
        ptr->read.cb = cb;
        ptr->read.sch = getThis();
    }
    if(event & WRITE) {
        ptr->write.fiber = Fiber::getThis();
        ptr->write.cb = cb;
        ptr->write.sch = getThis();
    }
    ptr->events = (curEvents | event);
    int op = curEvents ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    memset(&epevent, 0, sizeof(epoll_event));
    epevent.events = (ptr->events | EPOLLET);
    epevent.data.ptr = ptr;
    int rt = epoll_ctl(m_epollfds, op, fd, &epevent);
    if(rt != 0) {
        SEAICE_LOG_ERROR(logger) << "rt = " << rt
            << " errno = " << strerror(errno);
        SEAICE_ASSERT(false);
        return -1;
    }
    SEAICE_LOG_DEBUG(logger) << "add event = " <<
        event << " fd = " << fd;
    return 0;
}

void IOManager::delEvent(int fd, int event){
    if(event == NONE || fd < 0) {
        SEAICE_LOG_DEBUG(logger) <<"delEvent fd invalid " <<
            "or event is NONE";
        return;
    }

    RWMutexType::ReadLock lock(m_mutex);
    std::vector<FdContext>::size_type size = mFdCtxs.size();
    if((int)size < fd) {
        SEAICE_LOG_DEBUG(logger) <<"delEvent fd not exist";
        return;
    }
    FdContext* ptr = mFdCtxs[fd];
    lock.unlock();

    SEAICE_ASSERT(ptr);

    MutexType::Lock lock2(ptr->mutex);
    int curEvents = ptr->events;
    if((curEvents & event) == NONE) {
        return;
    }
    if(event & READ) {
        ptr->read.reset();
    }
    if(event & WRITE) {
        ptr->write.reset();
    }

    ptr->events = (curEvents & ~event);
    int op = ptr->events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    memset(&epevent, 0, sizeof(epoll_event));
    epevent.events = (ptr->events | EPOLLET);
    epevent.data.ptr = ptr;
    int rt = epoll_ctl(m_epollfds, op, fd, &epevent);
    if(rt != 0) {
        SEAICE_LOG_ERROR(logger) << "rt = " << rt
            << " errno = " << strerror(errno);
        SEAICE_ASSERT(false);
    }
}

bool IOManager::cancelEvent(int fd, int event) {
    if(event == NONE || fd < 0) {
        SEAICE_LOG_DEBUG(logger) <<"cancelEvent fd invalid " <<
            "or event is NONE";
        return false;
    }

    RWMutexType::ReadLock lock(m_mutex);
    if((int)mFdCtxs.size() < fd) {
        SEAICE_LOG_DEBUG(logger) <<"cancelEvent fd not exist";
        return false;
    }
    FdContext* ptr = mFdCtxs[fd];
    lock.unlock();

    SEAICE_ASSERT(ptr);

    MutexType::Lock lock2(ptr->mutex);
    int curEvents = ptr->events;
    if((curEvents & event) == NONE) {
        return false;
    }

    ptr->events = (curEvents & (~event));
    int op = ptr->events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    memset(&epevent, 0, sizeof(epoll_event));
    epevent.events = (ptr->events | EPOLLET);
    epevent.data.ptr = ptr;
    int rt = epoll_ctl(m_epollfds, op, fd, &epevent);
    if(rt != 0) {
        SEAICE_LOG_ERROR(logger) << "rt = " << rt
            << " errno = " << strerror(errno);
        SEAICE_ASSERT(false);
        return false;
    }

    if(event & READ) {
        ptr->triggerEvent(READ);
        ptr->read.reset();
    }
    if(event & WRITE) {
        ptr->triggerEvent(WRITE);
        ptr->write.reset();
    }
    return true;
}

bool IOManager::cancelAllEvent(int fd) {
    if(fd < 0) {
        SEAICE_LOG_DEBUG(logger) <<"cancelAllEvent fd invalid ";
        return false;
    }

    RWMutexType::ReadLock lock(m_mutex);
    if((int)mFdCtxs.size() < fd) {
        SEAICE_LOG_DEBUG(logger) <<"cancelAllEvent fd not exist";
        return false;
    }
    FdContext* ptr = mFdCtxs[fd];
    lock.unlock();

    SEAICE_ASSERT(ptr);

    MutexType::Lock lock2(ptr->mutex);
    int curEvents = ptr->events;
    if(curEvents == NONE) {
        return false;
    }

    ptr->events = NONE;
    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    memset(&epevent, 0, sizeof(epoll_event));
    epevent.events = NONE;
    epevent.data.ptr = ptr;
    int rt = epoll_ctl(m_epollfds, op, fd, &epevent);
    if(rt != 0) {
        SEAICE_LOG_ERROR(logger) << "rt = " << rt
            << " errno = " << strerror(errno);
        SEAICE_ASSERT(false);
        return false;
    }

    if(curEvents & READ) {
        ptr->triggerEvent(READ);
        ptr->read.reset();
    }
    if(curEvents & WRITE) {
        ptr->triggerEvent(WRITE);
        ptr->write.reset();
    }
    return true;
}

void IOManager::resizeFdContext(int fd) {
    RWMutex::ReadLock lock(m_mutex);
    if((int)mFdCtxs.size() > fd) {
        return;
    }
    lock.unlock();
    RWMutex::WriteLock lock2(m_mutex);
    mFdCtxs.resize(fd * 1.5);
    for(int i = 0; i < (int)mFdCtxs.size(); i++) {
        if(!mFdCtxs[i]) {
            mFdCtxs[i] = new FdContext;
            mFdCtxs[i]->fd = i;
            mFdCtxs[i]->events = NONE;
        }
    }
}

void IOManager::idleFun() {
    //https://blog.csdn.net/weixin_39956356/article/details/108547787
    const static uint64_t MAX_EVENTS = 1000;
    epoll_event* events = new epoll_event[MAX_EVENTS]();
    std::shared_ptr<epoll_event> smart_events(events, [=](epoll_event* ptr){
        delete[] ptr;
    });
    while(!isStoping()) {
        SEAICE_LOG_DEBUG(logger) << "idleFun start";
        int rt = 0;
        do{
            const static int TIMEOUT = 5000;//ms
            int timeout = std::min((int)getRecentTimerout(), TIMEOUT);
            rt = epoll_wait(m_epollfds, events, MAX_EVENTS, (int)timeout);
            if(rt < 0 && errno == EINTR) {
            } else {
                break;
            }
        }while(true);

        SEAICE_LOG_DEBUG(logger) << "epoll wait rt = " << rt;

        std::list<std::function<void()> > callbacks;
        listTimeroutCallback(callbacks);
        for(auto it : callbacks) {
           schedule(it);
        }

        for(int i = 0; i < rt; i++) {
            if(events[i].data.fd == m_tickFds[0]) {
                uint8_t dummy[256];
                while(read(m_tickFds[0], dummy, sizeof(dummy)) > 0);
                SEAICE_LOG_DEBUG(logger) << "read tick fd";
                continue;
            }

            epoll_event& epevent = events[i];
            int real_event = NONE;
            if(events[i].events & (EPOLLERR | EPOLLHUP)) {
                real_event = (READ | WRITE);
            }
            if(events[i].events & EPOLLIN) {
                real_event |= READ;
            }
            if(events[i].events & EPOLLOUT) {
                real_event |= WRITE;
            }

            FdContext* fd_ctx = (FdContext*)events[i].data.ptr;
            MutexType::Lock lock(fd_ctx->mutex);
            real_event &= fd_ctx->events;
            int left_event = (fd_ctx->events & (~real_event));
            int op = left_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            epevent.events = (left_event | EPOLLET);
            epevent.data.ptr = fd_ctx;
            int rt = epoll_ctl(m_epollfds, op, fd_ctx->fd, &epevent);
            if(rt != 0) {
                SEAICE_LOG_DEBUG(logger) << "real event = " << real_event
                    <<" fd events = " << fd_ctx->events << " epoll event = "
                    << events[i].events <<" left event = " << left_event;
            }
            if(real_event & READ) {
                fd_ctx->triggerEvent(READ);
            }
            if(real_event & WRITE) {
                fd_ctx->triggerEvent(WRITE);
            }
            lock.unlock();
        }
        SEAICE_LOG_DEBUG(logger) << "idleFun end yieldToReady";
        Fiber::yieldToReady();
    }
}

void IOManager::tick() {
    SEAICE_LOG_DEBUG(logger) << "write tick fd";
    write(m_tickFds[1], "T", 1);
}

bool IOManager::isStoping() {
    bool isStop = Scheduler::isStoping();
    return false;
}

void IOManager::insertTimerAtFrontCallback() {
    tick();
}

}