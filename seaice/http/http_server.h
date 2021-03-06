#ifndef __SEAICE_HTTP_SERVER_H__
#define __SEAICE_HTTP_SERVER_H__

#include "../tcpserver.h"
#include "http_session.h"
#include "servlet.h"

namespace seaice{
namespace http{

class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer(bool keepalive = false
        , seaice::IOManager* worker = seaice::IOManager::GetThis()
        , seaice::IOManager* accept_worker = seaice::IOManager::GetThis());

    ServletDispatch::ptr getDispatch() const {return m_dispatch;}
    void setDispatch(ServletDispatch::ptr v) {m_dispatch = v;};
protected:
    virtual void handleClient(Socket::ptr sock);
private:
    bool m_Keepalive;
    ServletDispatch::ptr m_dispatch;
};


}
}

#endif