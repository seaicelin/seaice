#include "fd_manager.h"
#include "hook.h"
#include "iomanager.h"
#include "log.h"
#include "socket.h"
#include "macro.h"
#include <netinet/tcp.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>

namespace seaice{

static Logger::ptr logger = SEAICE_LOGGER("system");

Socket::ptr Socket::CreateTCP(Address::ptr address) {
    Socket::ptr sock(new Socket(address->getFamily(), TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDP(Address::ptr address) {
    Socket::ptr sock(new Socket(address->getFamily(), UDP, 0));
    sock->newSock();
    sock->m_connected = true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket() {
    Socket::ptr sock(new Socket(Socket::IPv4, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6() {
    Socket::ptr sock(new Socket(Socket::IPv6, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket() {
    Socket::ptr sock(new Socket(Socket::IPv4, UDP, 0));
    sock->newSock();
    sock->m_connected = true;
    return sock;
}

Socket::ptr Socket::CreateUDPSocket6() {
    Socket::ptr sock(new Socket(Socket::IPv6, UDP, 0));
    sock->newSock();
    sock->m_connected = true;
    return sock;
}

Socket::ptr Socket::CreateUinxTCPSocket() {
    Socket::ptr sock(new Socket(Socket::UNIX, TCP, 0));
    return sock;
}
Socket::ptr Socket::CreateUinxUDPSocket() {
    Socket::ptr sock(new Socket(Socket::UNIX, UDP, 0));
    return sock;
}

Socket::Socket(int family, int type, int protocol)
    : m_family(family)
    , m_type(type)
    , m_protocol(protocol)
    , m_sock(-1)
    , m_connected(false)
    , m_localAddr(nullptr)
    , m_remoteAddr(nullptr) {
}

Socket::~Socket() {
    close();
}

uint64_t Socket::getSendTimerout() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
    if(ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(uint64_t ms) {
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

uint64_t Socket::getRecvTimeout() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
    if(ctx) {
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(uint64_t ms) {
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

bool Socket::getOption(int level, int option, void* result, socklen_t* len) {
    int rt = getsockopt(m_sock, level, option, result, (socklen_t*)len);
    if(rt != 0) {
        SEAICE_LOG_ERROR(logger) << "getOption sock = " << m_sock
            << " level = " << level << " option = " << option
            << " errno = " << errno << " errstr = " << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int option, const void* result, socklen_t len) {
    int rt = setsockopt(m_sock, level, option, result, (socklen_t)len);
    if(rt != 0) {
        SEAICE_LOG_ERROR(logger) << "setOption sock = " << m_sock
            << " level = " << level << " option = " << option
            << " errno = " << errno << " errstr = " << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept() {
    Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
    int newsock = ::accept(m_sock, NULL, NULL);
    if(newsock == -1) {
        SEAICE_LOG_ERROR(logger) << "accept(" << m_sock << ")" <<
            " errno = " << errno << " errno str = " << strerror(errno);
    }
    if(sock->init(newsock)) {
        return sock;
    }
    return nullptr;
}

bool Socket::bind(const Address::ptr addr) {
    if(!isValid()) {
        newSock();
        if(m_sock == -1) {
            return false;
        }
    }
    int rt = ::bind(m_sock, addr->getAddr(), addr->getAddrLen());
    if(rt == -1) {
        SEAICE_LOG_ERROR(logger) << "bind(" << m_sock << ")" <<
            " errno = " << errno << " errno str = " << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
    SEAICE_LOG_DEBUG(logger)  << "Socket::connect addr:" << addr->toString()
        << " timeout = " << timeout_ms;
    if(!isValid()) {
        newSock();
        if(m_sock == -1) {
            return false;
        }
    }
    if(addr->getFamily() != m_family) {
        SEAICE_LOG_ERROR(logger) << "connect sock family" <<
            m_family << " addr family " << addr->getFamily() <<
            " not equal, addr = " << addr->toString();
        return false;
    }
    m_remoteAddr = addr;
    if(timeout_ms == (uint64_t)-1) {
        if(::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
            SEAICE_LOG_ERROR(logger) << "connect(" << m_sock << ")" <<
                " errno = " << errno << " errno str = " << strerror(errno);
            close();
            return false;
        }
    } else {
        if(connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms) != 0) {
            SEAICE_LOG_ERROR(logger) << "connect(" << m_sock << ")" <<
                " timeout = " << timeout_ms <<
                " errno = " << errno << " errno str = " << strerror(errno);
            close();
            return false;
        }
    }
    m_connected = true;
    getRemoteAddress();
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog) {
    if(!isValid()) {
        SEAICE_LOG_ERROR(logger) << "listen ivalid socket";
        return false;
    }
    if(::listen(m_sock, backlog)) {
        SEAICE_LOG_ERROR(logger) << "listen errno = " << errno <<
            " errno str = " << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::close() {
    if(m_sock == 1 && !m_connected) {
        return true;
    }
    m_connected = false;
    if(m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
    }
    return false;
}

int Socket::send(const void* buffer, size_t len, int flags) {
    if(isConnected()) {
        return ::send(m_sock, buffer, len, flags);
    }
    return -1;
}

int Socket::send(const iovec* buffers, size_t len, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = len;
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::sendTo(const void* buffer, size_t len, const Address::ptr to, int flags) {
    if(isConnected()) {
        return ::sendto(m_sock, buffer, len, flags, to->getAddr(), to->getAddrLen());
    }
    return -1;
}

int Socket::sendTo(const iovec* buffers, size_t len, const Address::ptr to, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_name = to->getAddr();
        msg.msg_namelen = to->getAddrLen();
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = len;
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recv(void* buffer, size_t len, int flags) {
    if(isConnected()) {
        return ::recv(m_sock, buffer, len, flags);
    }
    return -1;
}

int Socket::recv(iovec* buffers, size_t len, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = len;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recvFrom(void* buffer, size_t len, Address::ptr from, int flags) {
    if(isConnected()) {
        socklen_t len = from->getAddrLen();
        return ::recvfrom(m_sock, buffer, len, flags, from->getAddr(), &len);
    }
    return -1;
}

int Socket::recvFrom(iovec* buffers, size_t len, Address::ptr from, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_name = from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = len;
        return ::recvmsg(m_sock, &msg, flags);
    }
}

Address::ptr Socket::getRemoteAddress() {
    if(m_remoteAddr) {
        return m_remoteAddr;
    }
    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address);
            break;
        case AF_INET6:
            result.reset(new IPv6Address);
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknowAddress());
    }

    socklen_t len = result->getAddrLen();
    if(getpeername(m_sock, result->getAddr(), &len)) {
        return Address::ptr(new UnknowAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(len);
    }
    m_remoteAddr = result;
    return m_remoteAddr;
}

Address::ptr Socket::getLocalAddress() {
    if(m_localAddr) {
        return m_localAddr;
    }
    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address);
            break;
        case AF_INET6:
            result.reset(new IPv6Address);
            break;
        case AF_UNIX:
            result.reset(new UnixAddress);
            break;
        default:
            result.reset(new UnknowAddress);
            break;
    }

    socklen_t len = result->getAddrLen();
    if(getsockname(m_sock, result->getAddr(), &len)) {
        return Address::ptr(new UnknowAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(len);
    }
    m_localAddr = result;
    return m_localAddr;
}

int Socket::getError() {
    int error = 0;
    socklen_t len = sizeof(error);
    if(!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        error = errno;
    }
    return error;
}


std::ostream& Socket::dump(std::ostream& os) const {
    os <<"[";
    os  <<"Socket sock = " << m_sock
        <<" is connected = " << m_connected
        <<" family = " << m_family
        <<" type = " << m_type
        <<" protocol = " << m_protocol;
    if(m_localAddr) {
        os << "local addr = " << m_localAddr->toString();
    }
    if(m_remoteAddr) {
        os << " remote addr = " << m_remoteAddr->toString();
    }
    os << "]";
    return os;
}

std::string Socket::toString() const {
    std::stringstream os;
    dump(os);
    return os.str();
}

bool Socket::cancelRead() {
    auto iom = (IOManager*)IOManager::getThis();
    return iom->cancelEvent(m_sock, IOManager::READ);
}

bool Socket::cancelWrite() {
    auto iom = (IOManager*)IOManager::getThis();
    return iom->cancelEvent(m_sock, IOManager::WRITE);
}

bool Socket::cancelAll() {
    auto iom = (IOManager*)IOManager::getThis();
    return iom->cancelAllEvent(m_sock);
}

bool Socket::cancelAccept() {
    auto iom = (IOManager*)IOManager::getThis();
    return iom->cancelEvent(m_sock, IOManager::READ);
}

void Socket::initSockOpt() {
    int val = 1;
    //1、一般来说，一个端口释放后会等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以被再次使用。
    //https://blog.csdn.net/u010144805/article/details/78579528
    setOption(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    //TCP连接中启用和禁用TCP_NODELAY有什么影响？
    //https://www.zhihu.com/question/42308970
    if(m_type == SOCK_STREAM) {
        setOption(IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
    }
}

void Socket::newSock() {
    m_sock = socket(m_family, m_type, m_protocol);
    if(m_sock == -1) {
        SEAICE_LOG_ERROR(logger) <<"socket(" << m_family <<
            ", " << m_type << ", " << m_protocol << ")" <<
            " errno = " << errno <<" err str = " << strerror(errno);
    }
    initSockOpt();
}

bool Socket::init(int sock) {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
    if(ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock = sock;
        m_connected = true;
        initSockOpt();
        getLocalAddress();
        getRemoteAddress();
        return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const Socket& socket) {
    return socket.dump(os);
}

namespace {
    /* SSL 库初始化，参看 ssl-server.c 代码 */
    struct SSL_Init {
        SSL_Init() {
            SSL_library_init(); //SSL库初始化
            OpenSSL_add_all_algorithms(); //载入 SSL 算法
            SSL_load_error_strings(); //载入SSL错误信息
        }
    };
}

static SSL_Init s_SSL_init;

SSLScoket::ptr SSLScoket::CreateTCP(Address::ptr address) {
    SSLScoket::ptr sock(new SSLScoket(address->getFamily(), TCP, 0));
    return sock;
}

SSLScoket::ptr SSLScoket::CreateTCPSocket() {
    SSLScoket::ptr sock(new SSLScoket(Socket::IPv4, TCP, 0));
    return sock;
}

SSLScoket::ptr SSLScoket::CreateTCPSocket6() {
    SSLScoket::ptr sock(new SSLScoket(Socket::IPv6, TCP, 0));
    return sock;
}

SSLScoket::SSLScoket(int family, int type, int protocol) 
    : Socket(family, type, protocol){
}

SSLScoket::~SSLScoket() {
}

bool  SSLScoket::bind(const Address::ptr addr) {
    return Socket::bind(addr);
}

SSLScoket::ptr SSLScoket::accept() {
    SSLScoket::ptr sock(new SSLScoket(m_family, m_type, m_protocol));
    int newsock = ::accept(m_sock, NULL, NULL);
    if(newsock == -1) {
        SEAICE_LOG_ERROR(logger) << "accept(" << m_sock << ")" <<
            " errno = " << errno << " errno str = " << strerror(errno);
    }
    sock->m_ctx = m_ctx; //load certificate 时候产生一个 ctx
    if(sock->init(newsock)) {
        m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
        SSL_set_fd(m_ssl.get(), m_sock);
        if(SSL_accept(m_ssl.get()) == 1) {
            return sock;
        }
    }
    return nullptr;
}

bool  SSLScoket::connect(const Address::ptr addr, uint64_t timeout_ms) {
    bool rt = Socket::connect(addr, timeout_ms);
    if(rt) {
        m_ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
        m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
        SSL_set_fd(m_ssl.get(), m_sock);
        rt = (SSL_connect(m_ssl.get()) == 1);
    }
    return rt;
}

bool SSLScoket::listen(int backlog) {
    return Socket::listen(backlog);
}

bool SSLScoket::close() {
    return Socket::close();
}

int SSLScoket::send(const void* buffer, size_t len, int flags) {
    if(m_ssl) {
        return SSL_write(m_ssl.get(), buffer, len);
    }
    return -1;
}

int SSLScoket::send(const iovec* buffers, size_t len, int flags) {
    if(!m_ssl) {
        return -1;
    }
    int total = 0;
    for(size_t i = 0; i < len; i++) {
        int tmp = SSL_write(m_ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
        if(tmp <= 0) {
            return tmp;
        }
        total += tmp;
        if(tmp != (int)buffers[i].iov_len) {
            break;
        }
    }
    return total;
}

int SSLScoket::sendTo(const void* buffer, size_t len, const Address::ptr to, int flags) {
    SEAICE_ASSERT(false);
    return -1;
}

int SSLScoket::sendTo(const iovec* buffers, size_t len, const Address::ptr to, int flags) {
    SEAICE_ASSERT(false);
    return -1;
}

int SSLScoket::recv(void* buffer, size_t len, int flags) {
    if(m_ssl) {
        return SSL_read(m_ssl.get(), buffer, len);
    }
    return -1;
}

int SSLScoket::recv(iovec* buffers, size_t len, int flags) {
    if(!m_ssl) {
        return -1;
    }
    int total = 0;
    for(size_t i = 0; i < len; i++) {
        int tmp = SSL_read(m_ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
        if(tmp <= 0) {
            return tmp;
        }
        total += tmp;
        if(tmp != (int)buffers[i].iov_len) {
            break;
        }
    }
    return total;
}

int SSLScoket::recvFrom(void* buffer, size_t len, Address::ptr from, int flags) {
    SEAICE_ASSERT(false);
    return -1;
}

int SSLScoket::recvFrom(iovec* buffers, size_t len, Address::ptr from, int flags) {
    SEAICE_ASSERT(false);
    return -1;
}

std::ostream& SSLScoket::dump(std::ostream& os) const {
    os << "[SSLScoket sock = " << m_sock
       << " is_connected = " << m_connected
       << " family = " << m_family
       << " type = " << m_type
       << " protocol = " << m_protocol;
    if(m_localAddr) {
        os << " local addr = " << m_localAddr->toString();
    }
    if(m_remoteAddr) {
        os << " remote addr = " << m_remoteAddr->toString();
    }
    os << "]";
    return os;
}

std::string SSLScoket::toString() const {
    stringstream ss;
    dump(ss);
    return ss.str();
}

bool SSLScoket::loadCertificates(const std::string& cert_file, const std::string& key_file) {
    m_ctx.reset(SSL_CTX_new(SSLv23_server_method()), SSL_CTX_free);
    if(SSL_CTX_use_certificate_file(m_ctx.get(), cert_file.c_str(), SSL_FILETYPE_PEM) != 1) {
        SEAICE_LOG_ERROR(logger) << "SSL_CTX_use_certificate_file(" << cert_file
            << ") failed";
        return false;
    }
    if(SSL_CTX_use_PrivateKey_file(m_ctx.get(), key_file.c_str(), SSL_FILETYPE_PEM) != 1) {
        SEAICE_LOG_ERROR(logger) << "SSL_CTX_use_PrivateKey_file(" << key_file
            << ") failed";
        return false;
    }
    if(SSL_CTX_check_private_key(m_ctx.get()) != 1) {
        SEAICE_LOG_ERROR(logger) << "SSL_CTX_check_private_key cert_file" << cert_file
         << "key_file = " << key_file << " failed";
        return false;
    }
    return true;
}

bool SSLScoket::init(int sock) {
    return Socket::init(sock);

}



}