#include "ws_connection.h"

namespace seaice{

std::pair<HttpResult::ptr, WSConnection::ptr> Create(const std::string& url
                                                    , uint64_t timeout_ms
                                                    , const std::map<std::string, std::string>& headers) {
    Uri::ptr uri = Uri::Create(url);
    if(!uri) {
        return std::make_pair(std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                    , nullptr, "invalid host = " + url), nullptr);
    }
    return Create(uri, timeout_ms, headers);
}

std::pair<HttpResult::ptr, WSConnection::ptr> Create( Uri::ptr uri
                                                , uint64_t timeout_ms
                                                , const std::map<std::string, std::string>& headers) {
    Address::ptr addr = uri->createAddress();
    SEAICE_LOG_DEBUG(logger) << addr->toString();
    if(!addr) {
        auto result = std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST
                    , nullptr, "invalid host = " + uri->getHost());
        return std::make_pair(result, nullptr);
    }
    Socket::ptr sock = Socket::CreateTCP(addr);
    if(!sock) {
        auto result std::make_shared<HttpResult>((int)HttpResult::Error::CREATE_SOCKET_ERROR
                    , nullptr, "create socket failed: " + addr->toString()
                    + " errno = " + std::to_string(errno)
                    + " errstr = " + std::string(strerror(errno)));
        return std::make_pair(result, nullptr);
    }
    if(!sock->connect(addr)) {
        auto result = std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL
                    , nullptr, "connect fail: " + addr->toString());
        return std::make_pair(result, nullptr);
    }
    sock->setRecvTimeout(timeout_ms);
    WSConnection::ptr conn = std::make_shared<WSConnection>(sock);
    HttpRequest::ptr req(new HttpRequest());
    req->setPath(uri->getPath());
    req->setQuery(uri->getQuery());
    req->setFragment(uri->getFragment());
    req->setMethod(HttpMethod::GET);

    bool has_conn = false;
    bool has_host = false;
    for(auto& i : headers) {
        if(strcasecmp(i.first.c_str(), "connection") == 0) {
            has_conn = true;
        }
        if(!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
            has_host = !i.second.empty();
        }
        req->setHeader(i.first, i.second);
    }
    if(!has_conn) {
        req->setHeader("Connection", "Upgrade");
    }
    if(!has_host) {
        req->setHeader("Host", uri->getHost());
    }

    req->setClose(false);
    req->setHeader("Upgrade", "websocket");
    req->setHeader("Sec-WebSocket-Version", "13");
    //sec web key 需要重新设计
    req->setHeader("Sec-WebSocket-Key", "w4v7O6xFTi36lq3RNcgctw==");

    int rt = conn->sendRequest(req);
    if(rt == 0) {
        auto result = std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER
                    , nullptr, "send request closed by peer: " + addr->toString());
        return std::make_pair(result, nullptr);
    }
    if(rt < 0) {
        auto result = std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR
                    , nullptr, "send request socket error errno = " + std::to_string(errno)
                    + " errstr = " + std::string(strerror(errno)));
        return std::make_pair(result, nullptr);
    }
    auto rsp = conn->recvResponse();
    if(!rsp) {
        auto result = std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT
                    , nullptr, "recv response timeout: " + addr->toString()
                    + " timeout_ms:" + std::to_string(timeout_ms));
        return std::make_pair(result, nullptr);
    }
    if(rsp->getStatus() != HttpStatus::SWITCHING_PROTOCOLS) {
        auto result std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_WEBSOCK_STATUS
                    , nullptr, "recv invalid websocket response status: "
                    + http::HttpStatusToString(rsp->getStatus()) + " addr = " + addr->toString());
        return std::make_pair(result, nullptr);
    }
    auto result = std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
    return std::make_pair(result, conn);
}

WSConnection::WSConnection(Socket::ptr sock, bool owner) 
    : HttpConnection(sock, owner) {
}

WSConnection::~WSConnection() {
}

int WSConnection::sendMessage(WSFrameMessage::ptr message, bool fin) {
    return WSSendMessage(this, message, true, fin);
}

WSFrameMessage::ptr WSConnection::recvMessage() {
    return WSRecvMessage(this);
}

int WSConnection::sendPint() {
    return WSSendPing(this);
}

int WSConnection::sendPong() {
    return WSSendPong(this);
}


}