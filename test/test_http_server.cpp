#include "test.h"
#include "../seaice/http/http_server.h"
#include "../seaice/http/http.h"
#include "../seaice/http/http_session.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

void run() {
    seaice::http::HttpServer::ptr server(new seaice::http::HttpServer);
    auto addr = seaice::Address::LookupAnyIPAddress("[0.0.0.0:8080]");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getDispatch();
    sd->addServlet("/seaice/xx", [](seaice::http::HttpRequest::ptr req
                                , seaice::http::HttpResponse::ptr rsp
                                , seaice::http::HttpSession::ptr session){
        rsp->setBody(req->toString());
        return 0;
    });
    sd->addGlobServlet("/seaice/*", [](seaice::http::HttpRequest::ptr req
                                , seaice::http::HttpResponse::ptr rsp
                                , seaice::http::HttpSession::ptr session){
        rsp->setBody("Glob:\r\n" + req->toString());
        return 0;
    });
    server->start();
}

int main() {
    seaice::IOManager iom(2);
    iom.start();
    iom.schedule(run);
    iom.stop();
}