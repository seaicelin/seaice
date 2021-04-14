#include "http_server.h"
#include "../log.h"

namespace seaice{
namespace http{

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

HttpServer::HttpServer(bool keepalive, seaice::IOManager* worker
    , seaice::IOManager* accept_worker) 
    : TcpServer(worker, accept_worker) 
    , m_Keepalive(keepalive) {
    m_dispatch.reset(new ServletDispatch);
}

void HttpServer::handleClient(Socket::ptr sock) {
    HttpSession::ptr session(new HttpSession(sock));
    do {
        auto req = session->recvRequest();
        if(!req) {
            SEAICE_LOG_WARN(logger) << "recv http request fail, errno = "
                << errno << " errstr = " << strerror(errno)
                << " client = " << *sock;
            break;
        }
        SEAICE_LOG_DEBUG(logger) << "req = " << *req;

        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_Keepalive));
        m_dispatch->handle(req, rsp, session);

        //rsp->setBody("hello world");
        session->sendResponse(rsp);
        SEAICE_LOG_DEBUG(logger) << "rsp = " << *rsp;
    } while(m_Keepalive);
    session->close();
}



}
}