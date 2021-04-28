#include <dlfcn.h>
#include "log.h"
#include "hook.h"
#include "iomanager.h"
#include "fiber.h"
#include "fd_manager.h"
#include "timer.h"
#include "macro.h"
#include <string.h>

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

namespace seaice {

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt) \

void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
    //is_inited = true;
//name:sleep -> sleep_f = (sleep_fun)dlsym(RTLD, "sleep")
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}



//声明一个全局类型
struct _HookIniter {
    _HookIniter() {
        hook_init();
#define ASSERTXX(name) SEAICE_ASSERT(name ## _f);
        HOOK_FUN(ASSERTXX);
#undef ASSERTXX
    }
};

void set_hook_enable(bool enable) {
    t_hook_enable = enable;
}

bool is_hook_enable() {
    return t_hook_enable;
}

void init_hook() {
    static _HookIniter hookInit;
}

}//seaice


struct timer_info{
    int cancelled = 0;
};

//右值：https://www.cnblogs.com/5iedu/p/7698710.html
//std::forward/完美转发：https://www.cnblogs.com/5iedu/p/11324772.html
template<typename Fun, typename... Args>
static ssize_t do_io(int fd, Fun fun, const char* hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args) {
    //SEAICE_LOG_DEBUG(SEAICE_LOGGER("system")) << "do_io " << hook_fun_name;
    if(!seaice::is_hook_enable()) {
        //std::cout<< "do_io " << hook_fun_name << " hook not enable" <<std::endl;
        return fun(fd, std::forward<Args>(args)...);
    }

    seaice::FdCtx::ptr ctx = seaice::FdMgr::GetInstance()->get(fd);
    if(!ctx) {
        //std::cout<< "do_io " << hook_fun_name << " ctx = nullptr" <<std::endl;
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {
        //std::cout<< "do_io " << hook_fun_name << " ctx isClose" <<std::endl;
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonBlock()) {
        std::cout<< "do_io " << hook_fun_name <<  " ctx not socket" <<std::endl;
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    SEAICE_LOG_DEBUG(logger) << hook_fun_name;
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    SEAICE_LOG_DEBUG(logger) << "do_io fun name " << hook_fun_name <<
         " n = " << n << " timeout = " << to;
    while(n == -1 && errno == EINTR) {
        n = fun(fd, std::forward<Args>(args)...);
    }
    SEAICE_LOG_DEBUG(logger) << "do_io " << hook_fun_name << 
            " fd = " << fd << " event = " << event <<" n =" << n;

    if(n <= 0){
        SEAICE_LOG_DEBUG(logger) << " errno = " << errno << " err str = " << strerror(errno);
    }

    if(n == -1 && errno == EAGAIN) {
        seaice::IOManager* iom = (seaice::IOManager*)seaice::IOManager::getThis();
            seaice::Timer::ptr timer(nullptr);
        if(to != (uint64_t)-1) {
            SEAICE_LOG_ERROR(logger) << hook_fun_name << "do_io addtimer fd = "
                    << fd << " timeout = " << to;
            std::weak_ptr<timer_info> winfo(tinfo);
            timer = iom->addTimer(to, [winfo, iom, fd, event](){
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (seaice::IOManager::EVENT)event);
            });
        }
        int rt = iom->addEvent(fd, (seaice::IOManager::EVENT)event);
        if(rt != 0) {
            SEAICE_LOG_ERROR(logger) << hook_fun_name << "do_io addEvent fd = "
                    << fd << " event = " << event;
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            SEAICE_LOG_DEBUG(logger) << "do_io " << hook_fun_name << 
                    " fd = " << fd << " yieldToHold ";
            seaice::Fiber::yieldToHold();
            if(timer) {
                timer->cancel();
            }
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
}


extern "C" {/*
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX*/

#define XX(name) name ## _fun name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX

//using namespace seaice;

/***
*EAGAIN is often raised when performing non-blocking I/O. It means "there is no data available right now, try again later"
What it means is less important. What it implies:
your system call failed
nothing happened (system calls are atomic, and this one just did not happen)
you could try it again (it could fail again, possibly with a different result)
or you could choose otherwise.
The whole thing about EAGAIN is that your process is not blocked inside the system call; it has the right to choose: either retry or do something useful.
***/
unsigned int sleep(unsigned int seconds) {

    if(!seaice::is_hook_enable()) {
        return sleep_f(seconds);
    }
    SEAICE_LOG_DEBUG(logger) << "hook sleep";
    seaice::IOManager* iom = (seaice::IOManager*)seaice::IOManager::getThis();
    SEAICE_ASSERT(iom);
    seaice::Fiber::ptr fiber = seaice::Fiber::getThis();
    iom->addTimer(seconds * 1000, [iom, fiber](){
        iom->schedule(fiber);
    });
    seaice::Fiber::yieldToReady();
    return 0;
}

int usleep(useconds_t usec) {
    if(!seaice::is_hook_enable()) {
        return usleep_f(usec);
    }
    if(usec <= 0) {
        return 0;
    }
    int ms = usec / 1000;
    SEAICE_LOG_DEBUG(logger) << "hook usleep";
    seaice::IOManager* iom = (seaice::IOManager*)seaice::IOManager::getThis();
    SEAICE_ASSERT(iom);
    seaice::Fiber::ptr fiber = seaice::Fiber::getThis();
    iom->addTimer(ms, [iom, fiber](){
        iom->schedule(fiber);
    });
    seaice::Fiber::yieldToReady();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!seaice::is_hook_enable()) {
        return nanosleep_f(req, rem);
    }
    int ms = (req->tv_sec * 1000) + (req->tv_nsec / 1000 / 1000);
    SEAICE_LOG_DEBUG(logger) << "hook nanosleep";
    seaice::IOManager* iom = (seaice::IOManager*)seaice::IOManager::getThis();
    SEAICE_ASSERT(iom);
    seaice::Fiber::ptr fiber = seaice::Fiber::getThis();
    iom->addTimer(ms, [iom, fiber](){
        iom->schedule(fiber);
    });
    seaice::Fiber::yieldToReady();
}

int socket(int domain, int type, int protocol) {
    if(!seaice::is_hook_enable()) {
        return socket_f(domain, type, protocol);
    }
    SEAICE_LOG_DEBUG(logger) << "hook socket";
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    seaice::FdMgr::GetInstance()->get(fd, true);
    return fd;
}


int connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, -1);
}


int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    SEAICE_LOG_DEBUG(logger) << "hook accept";
    int fd = do_io(sockfd, accept_f, "accept", seaice::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        seaice::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}


ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", seaice::IOManager::READ, SO_RCVTIMEO, buf, count);
}


ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", seaice::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}


ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", seaice::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}


ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", seaice::IOManager::READ, SO_RCVTIMEO,
                    buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", seaice::IOManager::READ, SO_RCVTIMEO,
                    msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", seaice::IOManager::WRITE, SO_SNDTIMEO,
                    buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", seaice::IOManager::WRITE, SO_SNDTIMEO,
                    iov, iovcnt);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    return do_io(sockfd, send_f, "send", seaice::IOManager::WRITE, SO_SNDTIMEO,
                    buf, len, flags);
}


ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
              const struct sockaddr *dest_addr, socklen_t addrlen) {
    return do_io(sockfd, sendto_f, "sendto", seaice::IOManager::WRITE, SO_SNDTIMEO,
                    buf, len, flags, dest_addr, addrlen);
}


ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    return do_io(sockfd, sendmsg_f, "sendmsg", seaice::IOManager::WRITE, SO_SNDTIMEO,
                    msg, flags);
}


int close(int fd) {
    if(!seaice::is_hook_enable()) {
        return close_f(fd);
    }
    seaice::FdCtx::ptr ctx = seaice::FdMgr::GetInstance()->get(fd);
    if(ctx) {
        auto iom = (seaice::IOManager*)seaice::IOManager::getThis();
        if(iom) {
            iom->cancelAllEvent(fd);
        }
        seaice::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}


int fcntl(int fd, int cmd, ... /* arg */ ) {
/*
    va_list va;
    va_start(va, cmd);
    void* arg = va_arg(va, void*);
    va_end(va);
*/
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_GETFL : 
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                if(!seaice::is_hook_enable()) {
                    return arg;
                }
                seaice::FdCtx::ptr ctx = seaice::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonBlock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_SETFL: 
            {
                int arg = va_arg(va, int);
                va_end(va);
                if(!seaice::is_hook_enable()) {
                    return fcntl_f(fd, cmd, arg);
                }
                seaice::FdCtx::ptr ctx = seaice::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonBlock(arg & O_NONBLOCK);
                if(ctx->getSysNonBlock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg & ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_SETLEASE: 
//        case F_ADD_SEALS: 
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif 
        case F_NOTIFY: 
        case F_DUPFD: 
        case F_SETSIG: 
        case F_SETOWN: 
        case F_DUPFD_CLOEXEC: 
        case F_SETFD: 
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK :
        case F_SETLKW :
        case F_GETLK :
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX :
        case F_SETOWN_EX :
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}


int ioctl(int fd, unsigned long request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);
    if(!seaice::is_hook_enable()) {
        return ioctl_f(fd, request, arg);
    }
    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg; //!!是为了把 int 转为 bool 类型
        seaice::FdCtx::ptr ctx = seaice::FdMgr::GetInstance()->get(fd);
        if(!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(fd, request, arg);
        }
        ctx->setUserNonBlock(user_nonblock);
    }
    return ioctl_f(fd, request, arg);
}


int getsockopt(int sockfd, int level, int optname,
              void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}


int setsockopt(int sockfd, int level, int optname,
              const void *optval, socklen_t optlen) {
    if(!seaice::is_hook_enable()) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(SOL_SOCKET == level) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            seaice::FdCtx::ptr ctx = seaice::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

int connect_with_timeout(int fd, const struct sockaddr* addr, 
            socklen_t addrlen, uint64_t timeout_ms) {
    if(!seaice::is_hook_enable()) {
        return connect_f(fd, addr, addrlen);
    }
    seaice::FdCtx::ptr ctx = seaice::FdMgr::GetInstance()->get(fd);
    if(!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }
    if(!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen);
    }
    int n = connect_f(fd, addr, addrlen);
    SEAICE_LOG_DEBUG(logger) << "hook connect n = " << n;
    if(n == 0) {
        return 0;
    } else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

    seaice::IOManager* iom = (seaice::IOManager*)seaice::IOManager::getThis();
    seaice::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);
    //-1 0xffffffffffffffff
    if(timeout_ms != (uint64_t)-1) {
        timer = iom->addTimer(timeout_ms, [winfo, iom, fd](){
            auto t = winfo.lock();
            if(!t || t->cancelled) {
                return;
            }
            t->cancelled = ETIMEDOUT;
            iom->cancelEvent(fd, seaice::IOManager::WRITE);
        });
    }
    int rt = iom->addEvent(fd, seaice::IOManager::WRITE);
    if(rt == 0) {
        SEAICE_LOG_DEBUG(logger) << "hook connect yieldToHold start hold";
        seaice::Fiber::yieldToHold();
        SEAICE_LOG_DEBUG(logger) << "hook connect yieldToHold end hold";
        if(timer) {
            timer->cancel();
        }
        if(tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if(timer) {
            timer->cancel();
        }
        SEAICE_LOG_ERROR(logger) << "connect addEvent( fd = " <<
                fd << ", WRITE) error";
    }
    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

}//extern
