#include "rock_connection.h"
#include "../socket.h"
#include "../address.h"
#include "../log.h"

namespace seaice {

static Logger::ptr logger = SEAICE_LOGGER("system");

RockConnection::RockConnection(Socket::ptr sock
         , IOManager* worker
         , IOManager* io_worker) 
    : RockStream(sock, worker, io_worker) {
}

RockConnection::~RockConnection() {
}

RockConnection::ptr RockConnection::Create(const std::string& url, uint64_t timeout_ms) {
    Uri::ptr uri = Uri::Create(url);
    if(!uri) {
        SEAICE_LOG_ERROR(logger) << "create uri error, url =-" << url;
        return nullptr;
    }
    Address::ptr addr = uri->createAddress();
    SEAICE_LOG_DEBUG(logger) << addr->toString();
    if(!addr) {
        SEAICE_LOG_ERROR(logger) << "uri create address error, uri =-" << uri->toString();
        return nullptr;
    }
    Socket::ptr sock = Socket::CreateTCP(addr);
    if(!sock) {
        SEAICE_LOG_ERROR(logger) << "create socket failed: " + addr->toString()
                    + " errno = " + std::to_string(errno)
                    + " errstr = " + std::string(strerror(errno));
        return nullptr;
    }
    if(sock->connect(addr) == false) {
        SEAICE_LOG_ERROR(logger) << "socket connect fail: " + addr->toString();
        sock->close();
        return nullptr;
    }
    sock->setRecvTimeout(timeout_ms);
    RockConnection::ptr conn(new RockConnection(sock));
    return conn;
}

}
