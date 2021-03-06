#ifndef __SEAICE_TCP_SERVER_H__
#define __SEAICE_TCP_SERVER_H__

#include <functional>
#include <memory>
#include <stdint.h>
#include <vector>
#include "noncopyable.h"
#include "socket.h"
#include "iomanager.h"
#include "config2.h"

namespace seaice {

struct TcpServerConf {
    typedef std::shared_ptr<TcpServerConf> ptr;

    std::vector<std::string> address;
    int keepalive = 0;
    int timeout = 1000 * 60;
    int ssl = 0;
    std::string id;
    std::string type = "http";
    std::string name = "unknow";
    std::string cert_file;
    std::string key_file;
    std::string accept_worker;
    std::string process_worker;
    std::map<std::string, std::string> args;

    bool isValid() const {
        return !address.empty();
    }

    bool operator==(const TcpServerConf& rhs) const {
        return  address == rhs.address
                && keepalive == rhs.keepalive
                && timeout == rhs.timeout
                && name == rhs.name
                && ssl == rhs.ssl
                && cert_file == rhs.cert_file
                && key_file == rhs.key_file
                && address == rhs.address
                && accept_worker == rhs.accept_worker
                && process_worker == rhs.process_worker
                && args == rhs.args
                && id == rhs.id
                && type == rhs.type;
    }

    std::string toString() const {
        stringstream ss;
        ss << "[keepalive = " << keepalive
           << ", timeout = " << timeout
           << ", name = " << name
           << ", address = "
           << ", accept_worker = " << accept_worker
           << ", process_worker = " << process_worker;
        for(auto& i : address) {
           ss << i << ",";
        }
        ss << "]";
        return ss.str();
    }
};

template<>
class LexicalCast<std::string, TcpServerConf> {
public:
    TcpServerConf operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        TcpServerConf conf;
        conf.id = node["id"].as<std::string>(conf.id);
        conf.ssl = node["ssl"].as<int>(conf.ssl);
        conf.type = node["type"].as<std::string>(conf.type);
        conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
        conf.key_file = node["key_file"].as<std::string>(conf.key_file);
        conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
        conf.timeout = node["timeout"].as<int>(conf.timeout);
        conf.name = node["name"].as<std::string>(conf.name);
        conf.accept_worker = node["accept_worker"].as<std::string>(conf.accept_worker);
        conf.process_worker = node["process_worker"].as<std::string>(conf.process_worker);
        conf.args = LexicalCast<std::string, 
                std::map<std::string, std::string> >()(node["args"].as<std::string>(""));
        if(node["address"].IsDefined()) {
            for(size_t i = 0; i < node["address"].size(); i++) {
                conf.address.push_back(node["address"][i].as<std::string>());
            }
        }
        return conf;
    }
};

template<>
class LexicalCast<TcpServerConf, std::string>
{
public:
    std::string operator()(const TcpServerConf& conf) {
        YAML::Node node;
        node["id"] = conf.id;
        node["type"] = conf.type;
        node["ssl"] = conf.ssl;
        node["cert_file"] = conf.cert_file;
        node["key_file"] = conf.key_file;
        node["keepalive"] = conf.keepalive;
        node["timeout"] = conf.timeout;
        node["name"] = conf.name;
        node["process_worker"] = conf.process_worker;
        node["args"] = YAML::Load(LexicalCast<std::map<std::string, std::string>,
                std::string>()(conf.args));
        node["accept_worker"] = conf.accept_worker;
        for(auto& i : conf.address) {
            node["address"].push_back(i);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


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
    TcpServerConf::ptr getConf() const {return m_conf;}
    void setConf(TcpServerConf::ptr v) {m_conf = v;}
    void setConf(TcpServerConf& v) {m_conf.reset(new TcpServerConf(v));}
protected:
    virtual void startAccept(Socket::ptr sock);
    virtual void handleClient(Socket::ptr sock);
protected:
    IOManager* m_worker;
    IOManager* m_acceptWorker;
    uint64_t m_readTimrout;
    std::string m_name;
    bool m_isStop;
    std::vector<Socket::ptr> m_sockets;
    std::string m_type = "tcp";
    bool m_ssl = false;
    TcpServerConf::ptr m_conf;
};

}
#endif