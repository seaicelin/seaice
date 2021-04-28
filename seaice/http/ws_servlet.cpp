#include "ws_servlet.h"

namespace seaice{

FunctionWSServlet::FunctionWSServlet(HandleCallback handle_cb
                    , ConnectCallback& connect_cb = nullptr
                    , CloseCallback close_cb = nullptr)
    : FunctionWSServlet("FunctionWSServlet")
    , m_handleCb(handle_cb)
    , m_connectCb(connect_cb)
    , m_closeCb(close_cb){
}

void FunctionWSServlet::onConnect(HttpRequest::ptr req
                        , WSSession::ptr session) {
    if(m_connectCb) {
        m_connectCb(req, session);
    }
}

void FunctionWSServlet::handle(HttpRequest::ptr req 
                    , WSFrameMessage::ptr message
                    , WSSession::ptr session) {
    if(m_handleCb) {
        m_handleCb(req, message, session);
    }
}

void FunctionWSServlet::onClose(HttpRequest::ptr req
                    , WSSession::ptr session) {
    if(m_closeCb) {
        m_closeCb(req, session);
    }
}

WSServletDispatcher::WSServletDispatcher() {
    m_name = "WSServletDispatcher";
}

void WSServletDispatcher::addServlet(const std::string& uri
            , FunctionWSServlet::HandleCallback handlecb
            , FunctionWSServlet::ConnectCallback connect_cb
            , FunctionWSServlet::CloseCallback close_cb) {
    ServletDispatch::addServlet(uri, 
                        std::make_shared<FunctionWSServlet>(handle_cb,
                            , connect_cb, close_cb));
}

void WSServletDispatcher::addGlobServlet(const std::string& uri
            , FunctionWSServlet::HandleCallback handlecb
            , FunctionWSServlet::ConnectCallback connect_cb
            , FunctionWSServlet::CloseCallback close_cb) {
    ServletDispatch::addGlobServlet(uri,
                        std::make_shared<FunctionWSServlet>(handlecb
                            , connect_cb, close_cb));
}

WSServlet::prt WSServletDispatcher::getServlet(const std::string& uri) {
    auto slt = getMatchServlet(uri);
    return std::dynamic_pointer_cast<WSServlet>(slt);
}


}