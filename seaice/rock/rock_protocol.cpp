#include "rock_protocol.h"
#include "../log.h"
#include "../Config2.h"
#include "../zlib_stream.h"
#include "../endian.h"

namespace seaice{

static Logger::ptr logger = SEAICE_LOGGER("system");

RockBody::RockBody() {}
RockBody::~RockBody() {}

bool RockBody::serializeToByteArray(ByteArray::ptr ba) {
    ba->writeStringVint(m_body);
    return true;
}

bool RockBody::parseFromByteArray(ByteArray::ptr ba) {
    m_body = ba->readStringVint();
    return true;
}

std::string RockRequest::toString() const {
    std::stringstream ss;
    ss  << "[RockRequest sn = " << m_sn
        << " cmd = " << m_cmd
        << " body.length = " << m_body.size()
        << "]";
    return ss.str();
}

const std::string& RockRequest::getName() const {
    static const std::string& s_name = "RockRequest";
    return s_name;
}

Message::MessageType RockRequest::getType() const {
    return Message::REQUEST;
}

bool RockRequest::serializeToByteArray(ByteArray::ptr ba)  {
    try{
        bool v = true;
        v &= Request::serializeToByteArray(ba);
        v &= RockBody::serializeToByteArray(ba);
        return v;
    } catch(...) {
        SEAICE_LOG_ERROR(logger) << "RockRequest serializeToByteArray error";
    }
    return false;
}

bool RockRequest::parseFromByteArray(ByteArray::ptr ba) {
    try{
        bool v = true;
        v &= Request::parseFromByteArray(ba);
        v &= RockBody::parseFromByteArray(ba);
        return v;
    } catch(...) {
        SEAICE_LOG_ERROR(logger) << "RockRequest parseFromByteArray error";
    }
    return false;
}

std::string RockResponse::toString() const {
    std::stringstream ss;
    ss  << "[RockResponse sn = " << m_sn
        << " cmd = " << m_cmd
        << " result = " << m_result
        << " result_msg = " << m_resultStr
        << " body.length = " << m_body.size()
        << "]";
    return ss.str();
}

const std::string& RockResponse::getName() const {
    static const std::string& s_name = "RockResponse";
    return s_name;
}

Message::MessageType RockResponse::getType() const {
    return Message::RESPONSE;
}


bool RockResponse::serializeToByteArray(ByteArray::ptr ba)  {
    try{
        bool v = true;
        v &= Response::serializeToByteArray(ba);
        v &= RockBody::serializeToByteArray(ba);
        return v;
    } catch(...) {
        SEAICE_LOG_ERROR(logger) << "RockResponse parseFromByteArray error";
    }
    return false;
}

bool RockResponse::parseFromByteArray(ByteArray::ptr ba) {
    try{
        bool v = true;
        v &= Response::parseFromByteArray(ba);
        v &= RockBody::parseFromByteArray(ba);
        return v;
    } catch(...) {
        SEAICE_LOG_ERROR(logger) << "RockResponse parseFromByteArray error";
    }
    return false;
}

std::string RockNotify::toString() const {
    std::stringstream ss;
    ss << "[RockNotify m_notify = " << m_notify
       << " body.length = " << m_body.size()
       << "]";
    return ss.str();
}

const std::string& RockNotify::getName() const {
    static const std::string& s_name = "RockNotify";
    return s_name;
}

Message::MessageType RockNotify::getType() const {
    return Message::NOTIFY;
}


bool RockNotify::serializeToByteArray(ByteArray::ptr ba) {
    try{
        bool v = true;
        v &= Notify::serializeToByteArray(ba);
        v &= RockBody::serializeToByteArray(ba);
        return v;
    } catch (...) {
        SEAICE_LOG_ERROR(logger) << "RockNotify serializeToByteArray error" ;
    }
    return false;
}

bool RockNotify::parseFromByteArray(ByteArray::ptr ba) {
    try{
        bool v = true;
        v &= Notify::parseFromByteArray(ba);
        v &= RockBody::parseFromByteArray(ba);
        return v;
    } catch(...) {
        SEAICE_LOG_ERROR(logger) << "RockNotify parseFromByteArray error";
    }
    return false;
}

RockMsgHeader::RockMsgHeader()
    : magic{0xab, 0xcd}
    , version(0x01)
    , flag(0)
    , length(0) {

}

static const uint8_t s_rock_magic[2] = {0xab, 0xcd};

static seaice::ConfigVar<uint32_t>::ptr g_rock_protocol_max_length =
    seaice::Config2::Lookup("rock.protocol.max_length", (uint32_t)(1024 * 1024 * 64)
                        , "rock protocol max length");

static seaice::ConfigVar<uint32_t>::ptr g_rock_protocol_min_length =
    seaice::Config2::Lookup("rock.protocol.min_length", (uint32_t)(1024 * 4)
                        , "rock protocol gzip min length");

static bool isRockMessageHeaderValid(RockMsgHeader& header) {
    if(memcmp(header.magic, s_rock_magic, sizeof(s_rock_magic))) {
        SEAICE_LOG_ERROR(logger) << "RockMessageDecoder head magic error";
        return false;
    }

    if(header.version != 0x01) {
        SEAICE_LOG_ERROR(logger) << "RockMessageDecoder head version != 0x01";
        return false;
    }

    header.length = byteswapOnLittleEndian(header.length);
    if((uint32_t)header.length >= g_rock_protocol_max_length->getValue()) {
        SEAICE_LOG_ERROR(logger) << "RockMessageDecoder head.length("
                                << header.length << ") >="
                                << g_rock_protocol_max_length->getValue();
        return false;
    }
    return true;
}

Message::ptr RockMessageDecoder::parseFrom(Stream::ptr stream) {
    try {
        RockMsgHeader header;
        if(stream->readFixSize(&header, sizeof(header)) <= 0) {
            SEAICE_LOG_ERROR(logger) << "RockMessageDecoder read header error";
            return nullptr;
        }

        if(isRockMessageHeaderValid(header) == false) {
            return nullptr;
        }

        seaice::ByteArray::ptr ba(new seaice::ByteArray);
        if(stream->readFixSize(ba, header.length) <= 0) {
            SEAICE_LOG_ERROR(logger) << "RockMessageDecoder read body fail len = "
                << header.length;
            return nullptr;
        }

        ba->setPos(0);
        if(header.flag & 0x01) {
            auto zstream = seaice::ZlibStream::CreateGzip(false);
            if(zstream->write(ba, ba->getSize()) != Z_OK) {
                SEAICE_LOG_ERROR(logger) << "RockMessageDecoder ungzip error";
                return nullptr;
            }
            if(zstream->flush() != Z_OK) {
                SEAICE_LOG_ERROR(logger) << "RockMessageDecoder ungzip flush error";
                return nullptr;
            }
            ba = zstream->getByteArray();
        }
        uint8_t type = ba->readFuint8();
        Message::ptr msg;
        switch(type) {
            case Message::REQUEST:
                msg.reset(new RockRequest);
                break;
            case Message::RESPONSE:
                msg.reset(new RockResponse);
                break;
            case Message::NOTIFY:
                msg.reset(new RockNotify);
                break;
            default:
                SEAICE_LOG_ERROR(logger) << "RockMessageDecoder invalid type = "
                    << (int)type;
                return nullptr;
        }
        if(!msg->parseFromByteArray(ba)) {
            SEAICE_LOG_ERROR(logger) << "RockMessageDecoder parseFromByteArray failed type = "
                << (int)type;
            return nullptr;
        }
        return msg;
    } catch (std::exception& e) {
        SEAICE_LOG_ERROR(logger) << "RockMessageDecoder except: " << e.what();
    } catch (...) {
        SEAICE_LOG_ERROR(logger) << "RockMessageDecoder except";
    }
    return nullptr;
}

int32_t RockMessageDecoder::serializeTo(Stream::ptr stream, Message::ptr msg) {
    RockMsgHeader header;
    auto ba = msg->toByteArray();
    ba->setPos(0);
    header.length = ba->getSize();
    if(header.length >= g_rock_protocol_min_length->getValue()) {
        auto zstream = seaice::ZlibStream::CreateGzip(true);
        if(zstream->write(ba, ba->getSize()) != Z_OK) {
            SEAICE_LOG_ERROR(logger) << "RockMessageDecoder serializeTo Gzip error";
            return -1;
        }
        if(zstream->flush() != Z_OK) {
            SEAICE_LOG_ERROR(logger) << "RockMessageDecoder flush error";
        }
        ba = zstream->getByteArray();
        header.flag |= 0x01;
        header.length = ba->getSize();
    }
    header.length = seaice::byteswapOnLittleEndian(header.length);
    if(stream->writeFixSize(&header, sizeof(header)) <= 0) {
        SEAICE_LOG_ERROR(logger) << "RockMessageDecoder serializeTo write header fail";
        return -3;
    }
    if(stream->writeFixSize(ba, ba->getReadSize()) <= 0) {
        SEAICE_LOG_ERROR(logger) << "RockMessageDecoder serializeTo write body fail";
        return -4;
    }
    return sizeof(header) + ba->getSize();
}

}