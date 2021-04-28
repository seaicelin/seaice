#ifndef __SEAICE_SOCKET_H__
#define __SEAICE_SOCKET_H__

#include "address.h"
#include <memory>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace seaice{

class Socket{
public:
    typedef std::shared_ptr<Socket> ptr;

    enum Type
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    enum Family
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX
    };

    static Socket::ptr CreateTCP(Address::ptr address);
    static Socket::ptr CreateUDP(Address::ptr address);
    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket();
    static Socket::ptr CreateUDPSocket6();
    static Socket::ptr CreateUinxTCPSocket();
    static Socket::ptr CreateUinxUDPSocket();

    Socket(int family, int type, int protocol);
    virtual ~Socket();

    uint64_t getSendTimerout();
    void setSendTimeout(uint64_t v);
    uint64_t getRecvTimeout();
    void setRecvTimeout(uint64_t v);
    bool getOption(int level, int option, void* result, socklen_t* len);
    bool setOption(int level, int option, const void* result, socklen_t len);
    Socket::ptr accept();
    virtual bool bind(const Address::ptr addr);
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
    virtual bool listen(int backlog = 1000);
    virtual bool close();
    virtual int send(const void* buffer, size_t len, int flags = 0);
    virtual int send(const iovec* buffers, size_t len, int flags = 0);
    virtual int sendTo(const void* buffer, size_t len, const Address::ptr to, int flags = 0);
    virtual int sendTo(const iovec* buffers, size_t len, const Address::ptr to, int flags = 0);
    virtual int recv(void* buffer, size_t len, int flags = 0);
    virtual int recv(iovec* buffers, size_t len, int flags = 0);
    virtual int recvFrom(void* buffer, size_t len, Address::ptr from, int flags = 0);
    virtual int recvFrom(iovec* buffers, size_t len, Address::ptr from, int flags = 0);
    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    int getSocket() const {return m_sock;}
    int getFamily() const {return m_family;}
    int getType() const {return m_type;}
    int getProtocol() const {return m_protocol;}
    bool isConnected() const {return m_connected;}
    bool isValid() const {return m_sock != -1;}
    int getError();
    virtual std::ostream& dump(std::ostream& os) const;
    virtual std::string toString() const;
    bool cancelRead();
    bool cancelWrite();
    bool cancelAll();
    bool cancelAccept();
protected:
    void initSockOpt();
    void newSock();
    virtual bool init(int sock);

protected:
    int m_family;
    int m_type;
    int m_protocol;
    int m_sock;
    bool m_connected;
    Address::ptr m_localAddr;
    Address::ptr m_remoteAddr;
};

std::ostream& operator<<(std::ostream& os, const Socket& socket);

class SSLScoket : public Socket {
public:
    typedef std::shared_ptr<SSLScoket> ptr;

    static SSLScoket::ptr CreateTCP(Address::ptr address);
    static SSLScoket::ptr CreateTCPSocket();
    static SSLScoket::ptr CreateTCPSocket6();

    SSLScoket(int family, int type, int protocol);
    virtual ~SSLScoket();

    virtual bool bind(const Address::ptr addr) override;
    SSLScoket::ptr accept();
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1) override;
    virtual bool listen(int backlog = 1000) override;
    virtual bool close() override;
    virtual int send(const void* buffer, size_t len, int flags = 0) override;
    virtual int send(const iovec* buffers, size_t len, int flags = 0) override;
    virtual int sendTo(const void* buffer, size_t len, const Address::ptr to, int flags = 0) override;
    virtual int sendTo(const iovec* buffers, size_t len, const Address::ptr to, int flags = 0) override;
    virtual int recv(void* buffer, size_t len, int flags = 0) override;
    virtual int recv(iovec* buffers, size_t len, int flags = 0) override;
    virtual int recvFrom(void* buffer, size_t len, Address::ptr from, int flags = 0) override;
    virtual int recvFrom(iovec* buffers, size_t len, Address::ptr from, int flags = 0) override;
    virtual std::ostream& dump(std::ostream& os) const;
    virtual std::string toString() const;

    bool loadCertificates(const std::string& cert_file, const std::string& key_file);
protected:
    virtual bool init(int sock) override;

private:
    std::shared_ptr<SSL>     m_ssl;
    std::shared_ptr<SSL_CTX> m_ctx;
};

}
#endif