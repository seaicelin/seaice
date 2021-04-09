#ifndef __SEAICE_TCP_SERVER_H__
#define __SEAICE_TCP_SERVER_H__

#include <functional>
#include <memory>
#include <stdint.h>
#include <vector>
#include "noncopyable.h"
#include "socket.h"
#include "iomanager.h"

namespace seaice {

class TcpServer : public std::enable_shared_from_this<TcpServer>
                    , Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(IOManager* worker = IOManager::GetThis(), 
                IOManager* acceptWorker = IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs, 
                        std::vector<Address::ptr>& fails);
    virtual bool start();
    virtual void stop();
    uint64_t getReadTimeout() const {return m_readTimrout;};
    std::string getName() const {return m_name;}
    void setReadTimeout(uint64_t v) {m_readTimrout = v;}
    void setName(const std::string& name) {m_name = name;}
    bool isStop() const {return m_isStop;}

protected:
    virtual void startAccept(Socket::ptr sock);
    virtual void handleClient(Socket::ptr sock);
private:
    IOManager* m_worker;
    IOManager* m_acceptWorker;
    uint64_t m_readTimrout;
    std::string m_name;
    bool m_isStop;
    std::vector<Socket::ptr> m_sockets;
};

}
#endif