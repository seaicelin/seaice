#include "ws_servlet.h"

namespace seaice {
namespace http {

WSServlet::WSServlet(const std::string& name) 
    : Servlet(name){
}

WSServlet::~WSServlet() {
}

WSFunctionServlet::WSFunctionServlet(HandleCallback handle_cb
                    , ConnectCallback connect_cb
                    , CloseCallback close_cb)
    : WSServlet("WSFunctionServlet")
    , m_handleCb(handle_cb)
    , m_connectCb(connect_cb)
    , m_closeCb(close_cb){
}

void WSFunctionServlet::onConnect(HttpRequest::ptr req
                        , WSSession::ptr session) {
    if(m_connectCb) {
        m_connectCb(req, session);
    }
}

void WSFunctionServlet::handle(HttpRequest::ptr req 
                    , WSFrameMessage::ptr message
                    , WSSession::ptr session) {
    if(m_handleCb) {
        m_handleCb(req, message, session);
    }
}

void WSFunctionServlet::onClose(HttpRequest::ptr req
                    , WSSession::ptr session) {
    if(m_closeCb) {
        m_closeCb(req, session);
    }
}

WSServletDispatcher::WSServletDispatcher() {
    m_name = "WSServletDispatcher";
}

void WSServletDispatcher::addServlet(const std::string& uri
            , WSFunctionServlet::HandleCallback handle_cb
            , WSFunctionServlet::ConnectCallback connect_cb
            , WSFunctionServlet::CloseCallback close_cb) {
    ServletDispatch::addServlet(uri, 
                        std::make_shared<WSFunctionServlet>(handle_cb
                            , connect_cb, close_cb));
}

void WSServletDispatcher::addGlobServlet(const std::string& uri
            , WSFunctionServlet::HandleCallback handle_cb
            , WSFunctionServlet::ConnectCallback connect_cb
            , WSFunctionServlet::CloseCallback close_cb) {
    ServletDispatch::addGlobServlet(uri,
                        std::make_shared<WSFunctionServlet>(handle_cb
                            , connect_cb, close_cb));
}

WSServlet::ptr WSServletDispatcher::getServlet(const std::string& uri) {
    auto slt = getMatchServlet(uri);
    return std::dynamic_pointer_cast<WSServlet>(slt);
}


}
}