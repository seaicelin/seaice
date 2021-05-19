#include "async_socket_stream.h"
#include "macro.h"
#include <functional>
#include "log.h"

namespace seaice {

static Logger::ptr logger = SEAICE_LOGGER("system");

void AsyncSocketStream::Ctx::rsp() {
    if(!fiber || !sch) {
        return;
    }
    sch->schedule(fiber);
}

AsyncSocketStream::AsyncSocketStream(Socket::ptr sock, IOManager* worker, IOManager* io_worker)
    : SocketStream(sock, true)
    , m_worker(worker)
    , m_io_worker(io_worker) {
}


void AsyncSocketStream::start() {
    startWrite();
    startRead();
}

void AsyncSocketStream::startWrite() {
    SEAICE_ASSERT(m_worker);
    SEAICE_ASSERT(m_io_worker);
    m_worker->schedule(std::bind(&AsyncSocketStream::runWrite, shared_from_this()));

}

void AsyncSocketStream::startRead() {
    SEAICE_ASSERT(m_worker);
    SEAICE_ASSERT(m_io_worker);
    m_worker->schedule(std::bind(&AsyncSocketStream::runRead, shared_from_this()));
}

void AsyncSocketStream::runWrite() {
    while(isConnected()) {
        m_fiber_sem.wait();
        std::list<Ctx::ptr> ctxs;
        {
            RWMutexType::ReadLock lock(m_list_mutex);
            ctxs.swap(m_list_ctxs);
        }
        for(auto& i : ctxs) {
            if(!i->doSend(shared_from_this())) {
                SEAICE_LOG_ERROR(logger) << "ctx doSend failed";
            }
        }
    }
}

void AsyncSocketStream::runRead() {
    while(isConnected()) {
        Ctx::ptr ctx = doRecv();
        if(ctx) {
            ctx->rsp();
        }
    }
}

void AsyncSocketStream::enqueue(Ctx::ptr ctx) {
    {
        RWMutexType::WriteLock lock(m_list_mutex);
        m_list_ctxs.push_back(ctx);
    }
    m_fiber_sem.notify();
}

void AsyncSocketStream::addCtx(Ctx::ptr ctx) {
    RWMutexType::WriteLock lock(m_map_mutex);
    m_map_ctxs.insert(std::make_pair(ctx->sn, ctx));
}

AsyncSocketStream::Ctx::ptr AsyncSocketStream::getCtx(uint32_t sn) {
    RWMutexType::ReadLock lock(m_map_mutex);
    auto ctx = m_map_ctxs[sn];
    return ctx;
}

AsyncSocketStream::Ctx::ptr AsyncSocketStream::getAndDelCtx(uint32_t sn) {
    RWMutexType::ReadLock lock(m_map_mutex);
    auto it = m_map_ctxs.find(sn);
        //auto ctx = m_map_ctxs[sn];
    if(it != m_map_ctxs.end()) {
        m_map_ctxs.erase(sn);
    }
    return it->second;
}


}