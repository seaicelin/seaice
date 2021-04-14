#include "test.h"
#include "../seaice/http/http_server.h"
#include "../seaice/http/http.h"
#include "../seaice/http/http_session.h"
#include "../seaice/http/http_connection.h"
#include "../seaice/uri.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

void test_pool() {
    seaice::http::HttpConnectionPool::ptr pool(
        new seaice::http::HttpConnectionPool(
            "0.0.0.0"
            , "", 8080, 10, 1000*30, 5, false));
    seaice::IOManager::GetThis()->addTimer(3000, [pool](){
        auto r = pool->doGet("/", 300);
    }, true);
}

void run() {

    auto addr = seaice::Address::LookupAnyIPAddress("[0.0.0.0:8080]");

    seaice::Socket::ptr sock = seaice::Socket::CreateTCP(addr);
    sock->connect(addr);
    seaice::http::HttpConnection::ptr conn(new seaice::http::HttpConnection(sock));
    seaice::http::HttpRequest::ptr req(new seaice::http::HttpRequest);
    SEAICE_LOG_DEBUG(logger) << "req:" << std::endl << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    if(!rsp) {
        SEAICE_LOG_ERROR(logger) << "recv response error";
        return;
    }
    SEAICE_LOG_DEBUG(logger) << "rsp:" << std::endl << *rsp;

}

void run2() {
    using namespace seaice;
    using namespace seaice::http;
    Uri::ptr uri = Uri::Create("foo://user@sylar.com:8042/over/there?name=ferret#nose");
    SEAICE_LOG_DEBUG(logger) << "uri = " << uri->toString();
    Uri::ptr uri2 = Uri::Create("http://0.0.0.0:8080/over/there?name=ferret#nose");
    SEAICE_LOG_DEBUG(logger) << "uri2 = " << uri2->toString() << " PORT = " << uri2->getPort();
    Address::ptr addr = uri2->createAddress();
    SEAICE_LOG_DEBUG(logger) << "uri2 addr = " << addr->toString();
    HttpResult::ptr res = HttpConnection::DoGet("http://0.0.0.0:8080/over/there?name=ferret#nose",
                    5000);
    SEAICE_LOG_DEBUG(logger) << res->toString();
    test_pool();
}

int main() {
    seaice::IOManager iom(2);
    iom.start();
    iom.schedule(run2);
    iom.stop();
}