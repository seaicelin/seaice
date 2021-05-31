#include "rock_server.h"
#include "rock_session.h"
#include "rock_protocol.h"
#include "../log.h"

namespace seaice {

static Logger::ptr logger = SEAICE_LOGGER("system");

void RockServer::handleClient(Socket::ptr sock) {
    //sock->setRecvTimeout(10000);
    RockSession::ptr session(new RockSession(sock));
    session->setRequestHandler([](RockRequest::ptr req, RockResponse::ptr rsp
                             , Stream::ptr stream) {
        rsp->setBody("setBody: seaice");
    });

    session->setNotifyHandler([](RockNotify::ptr, Stream::ptr){
        SEAICE_LOG_DEBUG(logger) << "receive RockNotify message";
    });
    session->start();
}

}