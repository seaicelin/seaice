#ifndef __SEAICE_WS_SESSION_H__
#define __SEAICE_WS_SESSION_H__

#include "http_session.h"
#include "../stream.h"

/*
1. 定义数据帧的消息头
2. 定义帧数据消息体
3. 定义发送消息的接口
4. ping 发送接口
5. pong 发送接口
6. 定义接收消息的接口
7. 定义 handleshake --> 用于和客户端协商建立 websocket
   7.1 主动接收请求，判断请求是否 websocket 请求消息
   7.2 应答客户端请求，发送回复确认消息 rsp
   7.3 返回 req
8 定义通用发送消息接口
9 定义通用发送 ping/pong 接口
*/
namespace seaice {

#pragma pack(1)
struct WSFrameHead {
    enum OPCODE
    {
        CONTINUE = 0,
        TEXT_FRAME = 1,
        BIN_FRAME = 2;
        CLOSE = 8;
        PING = 0X9;
        PONG = 0XA
    };
    uint32_t opcode: 4;
    bool fin: 1;
    bool rsv1: 1;
    bool rsv2: 1;
    bool rsv3: 1;
    uint32_t paload_len: 7;
    bool mask: 1;

    std::string toString() const;
};
#pragma pack()

class WSFrameMessage {
public:
    typedef std::shared_ptr<WSFrameMessage> ptr;

    WSSession(int opcode, const std::string& data = "") 
        : m_opcode(opcode)
        , m_data(data) {
    }
    ~WSSession() {}

    int getOpcode() const {return m_opcode;}
    const std::string& getData() const {return m_data;}
    uint64_t getDataLen() const {return m_data.size();}
    void setOpcode(int code) {m_opcode = code;}
    void setData(const std::string& data) {m_data = data;}
private:
    int m_opcode;
    std::string m_data;
};

class WSSession : public HttpSession {
public:
    typedef std::shared_ptr<WSSession> ptr;

    WSSession();
    ~WSSession();

    int sendMessage(WSFrameMessage::ptr message, bool fin = true);
    WSFrameMessage::ptr recvMessage();
    int sendPing();
    int sendPong();
    HttpRequest::ptr handleShake();
private:
}

WSFrameMessage::ptr WSRecvMessage(Stream* stream);
int WSSendMessage(Stream* stream, WSFrameMessage::ptr message, bool isClient; bool fin = true);
int WSSendPing(Stream* stream);
int WSSendPong(Stream* stream);

}

#endif