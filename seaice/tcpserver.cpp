#include "TcpServer.h"
#include "log.h"

namespace seaice{

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

TcpServer::TcpServer(IOManager* worker, IOManager* acceptWorker) 
    : m_worker(worker)
    , m_acceptWorker(acceptWorker)
    , m_readTimrout(3000)
    , m_name("TcpServer")
    , m_isStop(true) {
}

TcpServer::~TcpServer() {
    for(auto& sock : m_sockets) {
        sock->close();
    }
    m_sockets.clear();
}

bool TcpServer::bind(Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs, 
                    std::vector<Address::ptr>& fails) {
    for(auto& addr : addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);
        if(!sock->bind(addr)) {
            SEAICE_LOG_ERROR(logger) << "bind socket failed"
                <<" errno = " << errno << " errno str = "
                << strerror(errno) <<" addr = " << addr->toString();
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()) {
            SEAICE_LOG_ERROR(logger) << "listen socket failed"
                <<" errno = " << errno <<" errno str = "
                << strerror(errno) << " addr = " << addr->toString();
                fails.push_back(addr);
                continue;
        }
        m_sockets.push_back(sock);
    }
    if(!fails.empty()) {
        m_sockets.clear();
        return false;
    }
    for(auto& i : m_sockets) {
        SEAICE_LOG_DEBUG(logger) << "server bind success: " << *i;
    }
    return true;
}

bool TcpServer::start() {
    if(!m_isStop) {
        SEAICE_LOG_DEBUG(logger) << "TcpServer hava been started!";
        return true;
    }
    m_isStop = false;
    for(auto& sock : m_sockets) {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                                    shared_from_this(), sock));
    }
    return true;
}
void TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptWorker->schedule([this, self]() {
        for(auto& sock : m_sockets) {
            sock->cancelAll();
            sock->close();
        }
        m_sockets.clear();
    });
}

void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_isStop) {
        Socket::ptr client = sock->accept();
        if(client) {
            m_worker->schedule(std::bind(&TcpServer::handleClient, 
                                shared_from_this(), client));
        } else {
            SEAICE_LOG_ERROR(logger) << "accept errno = " << errno
                << " str errno = " << strerror(errno);
        }
    }
}

void TcpServer::handleClient(Socket::ptr sock) {
    SEAICE_LOG_DEBUG(logger) << "handleClient" << sock;
}
}