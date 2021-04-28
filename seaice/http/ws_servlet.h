#ifndef __SEAICE_WS_SERVLET_H__
#define __SEAICE_WS_SERVLET_H__

#include "servlet.h"
#include "ws_session.h"

/*
1. 定义websock 建立连接时的回调， on_connect
2. 定义处理 websock 请求的处理函数， handle
3. 定义 websock 关闭连接的回调， on_close
4. 定义 dispatcher
*/
namespace seaice{

class WSServlet : public Servlet
{
public:
    typedef std::shared_ptr<WSServlet> ptr;
    WSServlet(const std::string& name);
    ~WSServlet();

    virtual void onConnect(HttpRequest::ptr req, WSSession::ptr session) = 0;
    virtual void handle(HttpRequest::ptr req, WSFrameMessage::ptr message
                        , WSSession::ptr session) = 0;
    virtual void onClose(HttpRequest::ptr req, WSSession::ptr session) = 0;

    int32_t handle(seaice::http::HttpRequest::ptr request
                    , seaice::http::HttpResponse::ptr response
                    , seaice::http::HttpSession::ptr session) override {
        return 0;
    }
};


class FunctionWSServlet : public WSServlet {
public:
    typedef std::shared_ptr<FunctionWSServlet> ptr;
    typedef std::function<void(HttpRequest::ptr req
                            , WSSession::ptr session)> ConnectCallback;
    typedef std::function<void(HttpRequest::ptr req
                            , WSFrameMessage::ptr message
                            , WSSession::ptr session)> HandleCallback;
    typedef std::function<void(HttpRequest::ptr req
                            , WSSession::ptr session)> CloseCallback;

    FunctionWSServlet(HandleCallback handle_cb
                    , ConnectCallback& connect_cb = nullptr
                    , CloseCallback close_cb = nullptr);

    virtual void onConnect(HttpRequest::ptr req
                            , WSSession::ptr session) override;
    virtual void handle(HttpRequest::ptr req 
                        , WSFrameMessage::ptr message
                        , WSSession::ptr session) override;
    virtual void onClose(HttpRequest::ptr req
                        , WSSession::ptr session) override;
private:
    ConnectCallback m_connectCb;
    HandleCallback m_handleCb;
    CloseCallback m_closeCb;
};

class WSServletDispatcher : public ServletDispatch {
public:
    typedef std::shared_ptr<WSServletDispatcher> ptr;
    typedef RWMutex RWMutexType;

    WSServletDispatcher();
    void addServlet(const std::string& uri
                , FunctionWSServlet::HandleCallback handlecb
                , FunctionWSServlet::ConnectCallback connect_cb = nullptr;
                , FunctionWSServlet::CloseCallback close_cb = nullptr);
    void addGlobServlet(const std::string& uri
                , FunctionWSServlet::HandleCallback handlecb
                , FunctionWSServlet::ConnectCallback connect_cb = nullptr
                , FunctionWSServlet::CloseCallback close_cb = nullptr);
    WSServlet::prt getServlet(const std::string& uri);
}



}
#endif