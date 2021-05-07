#ifndef __SEAICE_WS_CONNECTION_H__
#define __SEAICE_WS_CONNECTION_H__

#include "http_connection.h"
#include "ws_session.h"

/*
1. 接收消息
2. 发送消息
3. ping
4. pong
5. 引入消息帧头/消息帧数据结构体
6. 定义创建 connection 的静态方法
*/
namespace seaice {
namespace http {

class WSConnection : public http::HttpConnection {
public:
    typedef std::shared_ptr<WSConnection> ptr;

    static std::pair<http::HttpResult::ptr, WSConnection::ptr> Create(const std::string& url
                                    , uint64_t timeout_ms
                                    , const std::map<std::string, std::string>& headers = {});
    static std::pair<http::HttpResult::ptr, WSConnection::ptr> Create(Uri::ptr uri
                                    , uint64_t timeout_ms
                                    , const std::map<std::string, std::string>& headers = {});

    WSConnection(Socket::ptr sock, bool owner = true);
    ~WSConnection();

    int sendMessage(WSFrameMessage::ptr message, bool fin = true);
    int sendMessage(std::string msg, int opcode = WSFrameHead::TEXT_FRAME, bool fin = true);
    WSFrameMessage::ptr recvMessage();
    int sendPint();
    int sendPong();

};


}
}
#endif