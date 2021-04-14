#ifndef __SEAICE_HTTP_CONNECTION_H__
#define __SEAICE_HTTP_CONNECTION_H__

#include "../SocketStream.h"
#include "../uri.h"
#include "../utils.h"
#include "../mutex.h"
#include "http.h"

#include <atomic>
#include <list>
#include <vector>
#include <stdint.h>

namespace seaice{
namespace http{

struct HttpResult{
    typedef std::shared_ptr<HttpResult> ptr;
    enum class Error
    {
        OK = 0,
        INVALID_URL = 1,
        INVALID_HOST = 2,
        CONNECT_FAIL = 3,
        SEND_CLOSE_BY_PEER = 4,
        SEND_SOCKET_ERROR = 5,
        TIMEOUT = 6,
        CREATE_SOCKET_ERROR = 7,
        POOL_GET_CONNECTION = 8,
        POOL_INVALID_CONNECTION = 9,
    };

    HttpResult(int _result
               , HttpResponse::ptr _rsp
               , const std::string& _error)
        : result(_result)
        , response(_rsp)
        , error(_error) {}

        int result;
        HttpResponse::ptr response;
        std::string error;

    std::string toString() const;
};

class HttpConnection : public SocketStream {
friend class HttpConnectionPool;
public:
    typedef std::shared_ptr<HttpConnection> ptr;
    HttpConnection(Socket::ptr sock, bool owner = true);

    HttpResponse::ptr recvResponse();
    int sendRequest(HttpRequest::ptr);

    static HttpResult::ptr DoGet(const std::string& url
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    static HttpResult::ptr DoGet(Uri::ptr uri
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    static HttpResult::ptr DoPost(const std::string& url
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    static HttpResult::ptr DoPost(Uri::ptr uri
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpMethod method
                            , const std::string& url
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpMethod method
                            , Uri::ptr uri
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpRequest::ptr req
                            ,Uri::ptr uri
                            , uint64_t timeout_ms);

private:
    uint64_t m_createTime = 0;
    uint64_t m_request = 0;
};

class HttpConnectionPool {
public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    typedef Mutex MutexType;


    static HttpConnectionPool::ptr Create(const std::string& url
                                          , const std::string& vhost
                                          , uint32_t max_size
                                          , uint32_t max_alive_time
                                          , uint32_t max_request);

    HttpConnectionPool(const std::string& host
                        , const std::string& vhost
                        , uint32_t port
                        , uint32_t max_size
                        , uint32_t max_alive_time
                        , uint32_t max_request
                        , bool is_https);

    ~HttpConnectionPool();

    HttpConnection::ptr getConnection();
    HttpResult::ptr doGet(const std::string& url
                          , uint64_t timeout_ms
                          , const std::map<std::string, std::string>& headers = {}
                          , const std::string& body = "");
    HttpResult::ptr doGet(const Uri::ptr uri
                          , uint64_t timeout_ms
                          , const std::map<std::string, std::string>& headers = {}
                          , const std::string& body = "");
    HttpResult::ptr doPost(const std::string& url
                          , uint64_t timeout_ms
                          , const std::map<std::string, std::string>& headers = {}
                          , const std::string& body = "");
    HttpResult::ptr doPost(const Uri::ptr uri
                          , uint64_t timeout_ms
                          , const std::map<std::string, std::string>& headers = {}
                          , const std::string& body = "");
    HttpResult::ptr doRequest(HttpMethod method
                              , const std::string& url
                              , uint64_t timeout_ms
                              , const std::map<std::string, std::string>& headers = {}
                              , const std::string& body = "");
    HttpResult::ptr doRequest(HttpMethod method
                              , Uri::ptr uri
                              , uint64_t timeout_ms
                              , const std::map<std::string, std::string>& headers = {}
                              , const std::string& body = "");
    HttpResult::ptr doRequest(HttpRequest::ptr req
                              , uint64_t timeout_ms);
private:
    static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);
private:
    std::string m_host;
    std::string m_vhost;
    uint32_t m_port;
    uint32_t m_maxSize;
    uint32_t m_maxAliveTime;
    uint32_t m_maxRequest;
    bool m_isHttps;

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<int32_t> m_total = {0};
};

}
}
#endif