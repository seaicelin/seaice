#ifndef _SEAICE_IOMANAGER_H_
#define _SEAICE_IOMANAGER_H_
#include "timer.h"
#include "scheduler.h"
#include "mutex.h"
#include <memory>
#include <functional>
#include <vector>


namespace seaice {

class IOManager : public Scheduler, public TimerManager {
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;
    typedef Mutex MutexType;
/*
EPOLLIN:1
EPOLLOUT:4
EPOLLRDHUP:8192
EPOLLPRI:2
EPOLLERR:8
EPOLLHUP:16
EPOLLET:-2147483648
EPOLLONESHOT:1073741824
*/

    enum EVENT
    {
        NONE  = 0X00,
        READ  = 0x01,
        WRITE = 0x04
    };

    struct FdContext {
        struct EventContext {
            EventContext() 
            : sch(nullptr)
            , cb(nullptr)
            , fiber(nullptr) {
            }

            void reset() {
                sch = nullptr;
                cb = nullptr;
                fiber = nullptr;
            }
            Scheduler* sch;
            std::function<void()> cb;
            Fiber::ptr fiber;
        };

        EventContext read;
        EventContext write;

        int events;
        int fd;
        MutexType mutex;

        void triggerEvent(int event);
        //EventContext& getEventCtx(EVENT event);
        //void setEventCtx(EventContext ctx);
    };

    IOManager(int threadCount, const std::string name = "UNKNOW", bool use_caller = false);
    ~IOManager();

    int addEvent(int fd, int event, std::function<void()> cb = nullptr);
    void delEvent(int fd, int event);
    bool cancelEvent(int fd, int event);
    bool cancelAllEvent(int fd);
    void resizeFdContext(int fd);
    void idleFun() override;
    void tick() override;
    bool isStoping() override;
    void insertTimerAtFrontCallback() override;
    static IOManager* GetThis();
private:
    std::vector<FdContext*> mFdCtxs;
    RWMutexType m_mutex;
    int m_tickFds[2];
    int m_epollfds;
};


}

#endif