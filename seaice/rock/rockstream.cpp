#include "rockstream.h"
#include "../log.h"

namespace seaice {

static Logger::ptr logger = SEAICE_LOGGER("system");

uint32_t RockStream::RockCtx::doSend(Stream::ptr stream) {
    auto rock_stream = std::dynamic_pointer_cast<RockStream>(stream);
    return rock_stream->m_decoder->serializeTo(rock_stream, req);
}


RockStream::RockStream(Socket::ptr sock
         , IOManager* worker
         , IOManager* io_worker)
    : AsyncSocketStream(sock, worker, io_worker)
    , m_decoder(new RockMessageDecoder) {
}

bool RockStream::sendMessage(Message::ptr msg) {
    RockCtx::ptr ctx(new RockCtx);
    ctx->req = msg;
    enqueue(ctx);
}

RockStream::Ctx::ptr RockStream::request(RockRequest::ptr req) {
    RockCtx::ptr ctx(new RockCtx);
    ctx->sn = req->getSn();
    ctx->fiber = Fiber::getThis();
    ctx->sch = Scheduler::getThis();
    ctx->req = req;
    addCtx(ctx);
    enqueue(ctx);
    Fiber::yieldToHold();
    auto rsp = getCtx(req->getSn());
    if(!rsp) {
        SEAICE_LOG_ERROR(logger) << "request error, no rsp";
        return nullptr;
    } else {
        SEAICE_LOG_DEBUG(logger) << "req sn = " << req->getSn() <<" success";
        return ctx;
    }
}

RockStream::Ctx::ptr RockStream::doRecv() {
    Message::ptr msg = m_decoder->parseFrom(shared_from_this());
    if(!msg) {
        SEAICE_LOG_ERROR(logger) << "parseFrom error";
        close();
        return nullptr;
    }
    SEAICE_LOG_DEBUG(logger) << "doRecv";
    Message::MessageType type = msg->getType();
    if(msg->getType() == Message::REQUEST) {
        auto req = std::dynamic_pointer_cast<RockRequest>(msg);
        uint32_t sn = req->getSn();
        RockResponse::ptr rsp(new RockResponse);
        rsp->setSn(sn);
        rsp->setResultStr("rsp");
        m_request_handler(req, rsp, shared_from_this());
        sendMessage(rsp);
    } else if(msg->getType() == Message::RESPONSE) {
        auto rsp = std::dynamic_pointer_cast<RockResponse>(msg);
        uint32_t sn = rsp->getSn();
        auto ctx = getAndDelCtx(sn);
        if(!ctx) {
            SEAICE_LOG_ERROR(logger) << "no req sn find, error response";
        }
        SEAICE_LOG_DEBUG(logger) << msg->toString();
    } else if(msg->getType() == Message::NOTIFY) {
        auto nty = std::dynamic_pointer_cast<RockNotify>(msg);
        m_notify_handler(nty, shared_from_this());
    } else {
        SEAICE_LOG_ERROR(logger) << "msg type error";
        return nullptr;
    }

}


}