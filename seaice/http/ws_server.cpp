#include "ws_server.h"
#include "ws_session.h"
#include "../log.h"

namespace seaice {
namespace http {

static Logger::ptr logger = SEAICE_LOGGER("system");

WSServer::WSServer(seaice::IOManager* worker
        , seaice::IOManager* accept_worker) 
    : TcpServer(worker, accept_worker) {
    m_dispatch.reset(new WSServletDispatcher);
    m_type = "websocket_server";
}

WSServer::~WSServer() {
}

void WSServer::handleClient(Socket::ptr sock) {
    WSSession::ptr session = std::make_shared<WSSession>(sock);
    do {
        auto req = session->handleShake();
        if(!req) {
            SEAICE_LOG_ERROR(logger) << "handle shake error";
            break;
        }
        if(!m_dispatch) {
            SEAICE_LOG_ERROR(logger) << "ws servlet dispatcher is null";
            break;
        }
        WSServlet::ptr slt = m_dispatch->getServlet(req->getPath());
        if(!slt) {
            SEAICE_LOG_ERROR(logger) << "ws servlet is null";
            break;
        }
        slt->onConnect(req, session);
        while(true) {
            auto wsMsg = session->recvMessage();
            if(!wsMsg) {
                break;
            }
            slt->handle(req, wsMsg, session);
        }
    } while(0);
    session->close();
}


}
}