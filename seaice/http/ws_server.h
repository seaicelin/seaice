#ifndef __SEAICE_WS_SERVER_H__
#define __SEAICE_WS_SERVER_H__

#include "../tcpserver.h"
#include "ws_session.h"
#include "ws_servlet.h"
/*
主要逻辑：
1. 处理新连接客户,创建 session，并调用 shakehandle
2. 连接建立成功后，返回 req 消息
3. 根据req请求路径，获取 servlte
4. servlet -> on_connect 回调
5. 进入主循环
{
    6  session 调用接收消息
    7. servlet->handle()
} 
8 结束主循环，servlet->onclose
9. session->close()
*/
namespace seaice {
namespace http {

class WSServer : public TcpServer
{
public:
    typedef std::shared_ptr<WSServer> ptr;
    WSServer(seaice::IOManager* worker = seaice::IOManager::GetThis()
            , seaice::IOManager* accept_worker = seaice::IOManager::GetThis());
    ~WSServer();

    WSServletDispatcher::ptr getDispatch() const {return m_dispatch;}
    void setDispatch(WSServletDispatcher::ptr v) {m_dispatch = v;};

protected:
    virtual void handleClient(Socket::ptr sock);

private:
    WSServletDispatcher::ptr m_dispatch;
};


}
}
#endif