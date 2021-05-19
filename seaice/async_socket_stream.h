#ifndef __SEAICE_ASYNC_SOCKET_STREAM_H__
#define __SEAICE_ASYNC_SOCKET_STREAM_H__

#include <list>
#include <unordered_map>
#include <memory>
#include "rock_protocol.h"
#include "mutex.h"
#include "iomanager.h"
#include "socketstream.h"

/*
1, 启动两个 fiber, 用以接收和发送消息。
2, 发送的请求消息需要保存上下文 Ctx，使用 map<sn, ctx>
    2.1，包括当前fiber，有回复时回到当前 fiber, 完成一次消息发送和响应。
    2.2，包括超时时间,超时则返回错误到当前 fiber, 完成一次消息发送和响应。
    2.3，包括序列号，用以记录当前的消息序列号
    2.4，保存之前使用的 scheduler，重新调度 fiber
3, 发送回复消息使用 list《ctx》 保存，仅仅发送。
4, list,锁，map， 锁
5. 
*/

namespace seaice{

class AsyncSocketStream : public SocketStream
                        , public std::enable_shared_from_this<AsyncSocketStream> {
public:
    typedef std::shared_ptr<AsyncSocketStream> ptr;
    typedef RWMutex RWMutexType;

public:
struct Ctx {
    typedef std::shared_ptr<Ctx> ptr;

    Fiber::ptr fiber;
    Scheduler* sch;
    uint64_t timeoutMs;
    uint32_t sn;

    void rsp();
    virtual uint32_t doSend(Stream::ptr stream) = 0;
};

AsyncSocketStream(Socket::ptr sock, IOManager* worker = IOManager::GetThis()
                , IOManager* io_worker = IOManager::GetThis());
~AsyncSocketStream() {}

public:
    void start();
private:
    void startWrite();
    void startRead();
    void runWrite();
    void runRead();

protected:
    virtual Ctx::ptr doRecv() = 0;
    void enqueue(Ctx::ptr ctx);
    void addCtx(Ctx::ptr ctx);
    Ctx::ptr getCtx(uint32_t sn);
    Ctx::ptr getAndDelCtx(uint32_t sn);

private:
    IOManager* m_worker;
    IOManager* m_io_worker;
    RWMutexType m_list_mutex;
    RWMutexType m_map_mutex;
    std::unordered_map<uint32_t, Ctx::ptr> m_map_ctxs;
    std::list<Ctx::ptr> m_list_ctxs;
    FiberSemaphore m_fiber_sem;
};

}
#endif