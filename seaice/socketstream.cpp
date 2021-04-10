#include "socketstream.h"

namespace seaice{

SocketStream::SocketStream(Socket::ptr sock, bool owner) 
    : m_socket(sock)
    , m_owner(owner) {
}

SocketStream::~SocketStream() {
    if(m_owner && m_socket) {
        m_socket->close();
    }
}

int SocketStream::read(void* buffer, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    return m_socket->recv(buffer, length);
}

int SocketStream::read(ByteArray::ptr ba, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    std::vector<iovec> buffers;
    ba->getWriteBuffers(buffers,length);
    int rt = m_socket->recv(&buffers[0], buffers.size());
    if(rt > 0) {
        ba->setPos(ba->getPos() + rt);
    }
    return rt;
}

int SocketStream::write(const void* buffer, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    return m_socket->send(buffer, length);
}

int SocketStream::write(ByteArray::ptr ba, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    std::vector<iovec> buffers;
    ba->getReadBuffers(buffers,length);
    int rt = m_socket->send(&buffers[0], buffers.size());
    if(rt > 0) {
        ba->setPos(ba->getPos() + rt);
    }
    return rt;
}

void SocketStream::close() {
    if(m_socket) {
        m_socket->close();
    }
}

bool SocketStream::isConnected() const{
    return m_socket && m_socket->isConnected();
}

}