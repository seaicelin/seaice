#include "protocol.h"
#include "bytearray.h"

namespace seaice {


//Message::Message() {}
Message::~Message() {
}

ByteArray::ptr Message::toByteArray() {
    ByteArray::ptr ba(new ByteArray);
    if(serializeToByteArray(ba)) {
        return ba;
    }
    return nullptr;
}

Request::Request()
    : m_sn(0)
    , m_cmd(0) {
}

Request::~Request() {
}

bool Request::serializeToByteArray(ByteArray::ptr ba) {
    ba->writeFuint8(getType());
    ba->writeUint32(m_sn);
    ba->writeUint32(m_cmd);
    return true;
}

bool Request::parseFromByteArray(ByteArray::ptr ba) {
    m_sn = ba->readUint32();
    m_cmd = ba->readUint32();
    return true;
}

Response::Response() 
    : m_sn(0)
    , m_cmd(0)
    , m_result(0)
    , m_resultStr("unknow") {
}

Response::~Response() {}

bool Response::serializeToByteArray(ByteArray::ptr ba) {
    ba->writeFuint8(getType());
    ba->writeUint32(m_sn);
    ba->writeUint32(m_cmd);
    ba->writeUint32(m_result);
    ba->writeStringVint(m_resultStr);
    return true;
}

bool Response::parseFromByteArray(ByteArray::ptr ba) {
    m_sn = ba->readUint32();
    m_cmd = ba->readUint32();
    m_result = ba->readUint32();
    m_resultStr = ba->readStringVint();
    return true;
}

Notify::Notify() 
    : m_notify(0) {
}

Notify::~Notify() {
}

bool Notify::serializeToByteArray(ByteArray::ptr ba) {
    ba->writeFuint8(getType());
    ba->writeUint32(m_notify);
    return true;
}

bool Notify::parseFromByteArray(ByteArray::ptr ba) {
    m_notify = ba->readUint32();
    return true;
}

}