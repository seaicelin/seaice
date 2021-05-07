#include "../log.h"
#include "../endian.h"
#include "ws_session.h"
#include "../config2.h"
#include <sstream>
#include <string.h>

namespace seaice {
namespace http {

static Logger::ptr logger = SEAICE_LOGGER("system");

seaice::ConfigVar<uint32_t>::ptr g_websocket_message_max_size
    = seaice::Config2::Lookup("websocket.message.max_size"
        , (uint32_t) 1024 * 1024 * 32, "websocket message max size");

std::string WSFrameHead::toString() const {
    stringstream ss;
    ss << "[" 
       << "opcode = " << opcode
       << " fin = " << fin
       << " rsv1 = " << rsv1
       << " rsv2 = " << rsv2
       << " rsv3 = " << rsv3
       << " payload len = " << payload_len
       << " mask = " << mask
       << "]";
    return ss.str();
}

WSSession::WSSession(Socket::ptr sock) 
    : HttpSession(sock) {
}

WSSession::~WSSession() {
}

int WSSession::sendMessage(WSFrameMessage::ptr message, bool fin) {
    return WSSendMessage(this, message, false, fin);
}

WSFrameMessage::ptr WSSession::recvMessage() {
    return WSRecvMessage(this);
}

int WSSession::sendPing() {
    return WSSendPing(this);
}

int WSSession::sendPong() {
    return WSSendPong(this);
}

HttpRequest::ptr WSSession::handleShake() {
    auto req = recvRequest();
    if(!req) {
        SEAICE_LOG_ERROR(logger) << "handleShake recvRequest error";
    }
    if(strcasecmp(req->getHeader("Connection").c_str(), "Upgrade") != 0) {
        SEAICE_LOG_ERROR(logger) << "handleShake connection != Upgrade";
        return nullptr;
    }
    if(strcasecmp(req->getHeader("Upgrade").c_str(), "websocket") != 0) {
        SEAICE_LOG_ERROR(logger) << "handleShake Upgrade != websocket";
        return nullptr;
    }
    if(req->getHeaderAs<int>("Sec-WebSocket-Version") != 13) {
        SEAICE_LOG_ERROR(logger) << "handleShake Sec-WebSocket-Version != 13";
        return nullptr;
    }
    req->setWebSock(true);
    auto webScoketKey = req->getHeader("Sec-WebSocket-Key");
    std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose()));
    rsp->setStatus(HttpStatus::SWITCHING_PROTOCOLS);
    rsp->setReason("Web socket protocol handleShake");
    rsp->setHeader("Connection", "Upgrade");
    rsp->setHeader("Upgrade", "websocket");
    rsp->setHeader("Sec-WebSocket-Accept", "xx");
    //    rsp->setHeader("Sec-WebSocket-Version", "websocket");
    sendResponse(rsp);
    SEAICE_LOG_INFO(logger) << *req;
    SEAICE_LOG_INFO(logger) << *rsp;
    return req;
}

WSFrameMessage::ptr WSRecvMessage(Stream* stream, bool client) {
    int cur_len = 0;
    std::string data;
    int opcode = 0;
    do {
        WSFrameHead head;
        memset(&head, 0, sizeof(head));
        int rt = stream->readFixSize(&head, sizeof(head));
        if(rt <= 0) {
            SEAICE_LOG_ERROR(logger) << "recv Message WSFrameHead error";
            break;
        }
        SEAICE_LOG_INFO(logger) << head.toString();
        if(head.opcode == WSFrameHead::PING) {
            rt = WSSendPong(stream);
            if(rt <= 0) {
                SEAICE_LOG_ERROR(logger) << "recv message send pong error";
                break;
            }
        } else if(head.opcode == WSFrameHead::PONG){
        } else if(head.opcode == WSFrameHead::CONTINUE
                    || head.opcode == WSFrameHead::TEXT_FRAME
                    || head.opcode == WSFrameHead::BIN_FRAME) {
            //ws frame msg from client the mask should set 1
            if(head.mask != 1 && !client) {
                SEAICE_LOG_ERROR(logger) << "recv WSFrameHead mask != 1";
                break;
            }
            uint64_t dataLen = 0;
            if(head.payload_len == 126) {
                uint16_t len = 0;
                rt = stream->readFixSize(&len, sizeof(len));
                if(rt <= 0) {
                    SEAICE_LOG_ERROR(logger) << "recv message read data len error";
                    break;
                }
                dataLen = seaice::byteswapOnLittleEndian(len);
            } else if(head.payload_len == 127) {
                uint64_t len = 0;
                rt = stream->readFixSize(&len, sizeof(len));
                if(rt <= 0) {
                    SEAICE_LOG_ERROR(logger) << "recv message read data len error";
                    break;
                }
                dataLen = seaice::byteswapOnLittleEndian(len);
            } else {
                dataLen = head.payload_len;
            }

            if(dataLen > g_websocket_message_max_size->getValue()) {
                SEAICE_LOG_ERROR(logger) << "WSFrameMessage length: " << dataLen
                    << " > max size:" << g_websocket_message_max_size->getValue();
                break;
            }

            char mask[4] = {0};
            if(head.mask) {
                rt = stream->readFixSize(mask, 4);
                if(rt <= 0) {
                    SEAICE_LOG_ERROR(logger) << "recv message mask error";
                    break;
                }
            }

            data.resize(cur_len + dataLen);
            rt = stream->readFixSize(&data[cur_len], dataLen);
            if(rt <= 0) {
                SEAICE_LOG_ERROR(logger) << "recv message data error";
                break;
            }

            if(head.mask) {
                for(int i = 0; i < (int)dataLen; ++i) {
                    data[cur_len + i] ^= mask[i % 4];
                }
            }

            cur_len += dataLen;

            if(!opcode && head.opcode != WSFrameHead::CONTINUE) {
                opcode = head.opcode;
            }

            if(head.fin) {
                SEAICE_LOG_INFO(logger) << "data = " << data;
                return std::make_shared<WSFrameMessage>(opcode, std::move(data));
            }
        }
    } while(true);
    stream->close();
    return nullptr;
}

int WSSendMessage(Stream* stream, WSFrameMessage::ptr message, bool isClient, bool fin) {
    do {
        WSFrameHead head;
        memset(&head, 0, sizeof(head));
        head.fin = fin;
        head.opcode = message->getOpcode();
        if(isClient) {
            head.mask = true;
        }

        uint64_t dataLen = message->getDataLen();
        if(dataLen < 126) {
            head.payload_len = dataLen;
        } else if(dataLen <= 65535 ) {
            head.payload_len = 126;
        } else {
            head.payload_len = 127;
        }

        int rt = stream->writeFixSize(&head, sizeof(head));
        if(rt <= 0) {
            SEAICE_LOG_ERROR(logger) << "send message write head error";
            break;
        }
        if(head.payload_len == 126) {
            uint16_t len = (uint16_t)dataLen;
            len = seaice::byteswapOnLittleEndian(len);
            rt = stream->writeFixSize(&len, sizeof(len));
        } else if(head.payload_len == 127) {
            uint64_t len = dataLen;
            len = seaice::byteswapOnLittleEndian(len);
            rt = stream->writeFixSize(&len, sizeof(len));
        }
        if(rt <= 0) {
            SEAICE_LOG_ERROR(logger) << "send message write dataLen error";
            break;
        }

        if(head.mask && isClient) {
            char mask[4] = {0};
            uint32_t rand_value = rand();
            memcpy(mask, & rand_value, sizeof(mask));
            std::string& data = message->getData();
            for(size_t i = 0; i < data.size(); ++i) {
                data[i] ^= mask[i % 4];
            }
            rt = stream->writeFixSize(mask, sizeof(mask));
            if(rt <= 0) {
                SEAICE_LOG_ERROR(logger) << "send message mask error";
                break;
            }
        }

        rt = stream->writeFixSize(message->getData().c_str(), dataLen);
        if(rt <= 0) {
            SEAICE_LOG_ERROR(logger) << "send message write message data error";
            break;
        }
        return dataLen + sizeof(head);
    }while(0);
    return -1;
}

int WSSendPing(Stream* stream) {
    WSFrameHead head;
    memset(&head, 0, sizeof(head));
    head.fin = true;
    head.opcode = 0xA;
    int rt = stream->writeFixSize(&head, sizeof(head));
    if(rt <= 0) {
        SEAICE_LOG_ERROR(logger) << "send ping error";
    }
    return rt;
}

int WSSendPong(Stream* stream) {
    WSFrameHead head;
    memset(&head, 0, sizeof(head));
    head.fin = true;
    head.opcode = 0xA;
    int rt = stream->writeFixSize(&head, sizeof(head));
    if(rt <= 0) {
        SEAICE_LOG_ERROR(logger) << "send ping error";
    }
    return rt;
}


}
}