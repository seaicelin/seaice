#ifndef __SEAICE_ROCK_STREAM_H__
#define __SEAICE_ROCK_STREAM_H__

#include "async_socket_stream.h"

namespace seaice {

class RockStream : public AsyncSocketStream {
public:
    typedef std::shared_ptr<RockStream> ptr;
    typedef std::function<void(RockRequest::ptr, RockResponse::ptr
                             , Stream::ptr)> request_handler;
    typedef std::function<void(RockNotify::ptr, Stream::ptr)> notify_handler;

public:
    struct RockCtx : public Ctx {
        typedef std::shared_ptr<RockCtx> ptr;

        RockRequest::ptr req;
        RockResponse::ptr rsp;

        virtual uint32_t doSend(Stream::ptr stream) override;
    };

    RockStream(Socket::ptr sock
             , IOManager* worker = IOManager::GetThis()
             , IOManager* io_worker = IOManager::GetThis());
    ~RockStream() {}

    bool sendMessage(Message::ptr msg);
    Ctx::ptr request(RockRequest::ptr req);

    request_handler getRequestHandler() const {return m_request_handler;}
    notify_handler getNotifyHandler() const {return m_notify_handler;}
    void setRequestHandler(request_handler req) {m_request_handler = req;}
    void setNotifyHandler(notify_handler nty) {m_notify_handler = nty;}

    virtual Ctx::ptr doRecv() override;

protected:
    RockMessageDecoder::ptr m_decoder;
    request_handler m_request_handler;
    notify_handler m_notify_handler;
};

}
#endif