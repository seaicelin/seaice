#include "test.h"
#include "../seaice/http/http.h"
#include "../seaice/http/http_parser.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("seaice");

void test_req() {
    seaice::http::HttpRequest::ptr req(new seaice::http::HttpRequest);
    req->setHeader("host", "www.seaice.com");
    req->setBody("hello world");

    req->dump(std::cout) << std::endl;
}

void test_res() {
    seaice::http::HttpResponse::ptr res(new seaice::http::HttpResponse);
    res->setHeader("x-x", "header");
    res->setBody("response");
    res->setStatus(seaice::http::HttpStatus::NOT_FOUND);
    res->setClose(false);
    res->dump(std::cout) << std::endl;
}

const char test_request_data[] = "GET / HTTP/1.1\r\n"
                                "Host: www.baidu.com\r\n"
                                "Content-Length: 10\r\n\r\n"
                                "1234567890";

void test_http_parser() {
    seaice::http::HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    SEAICE_LOG_DEBUG(logger) << " s = " << s
        <<" has error = " << parser.hasError()
        <<" is finish = " << parser.isFinish()
        <<" total size = " << tmp.size()
        <<" Content-Length = " << parser.getContentLength();
    SEAICE_LOG_DEBUG(logger) << parser.getData()->toString();
    tmp.resize(tmp.size() - s);
    SEAICE_LOG_DEBUG(logger) << tmp;
}

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
        "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
        "Server: Apache\r\n"
        "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
        "ETag: \"51-47cf7e6ee8400\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: 81\r\n"
        "Cache-Control: max-age=86400\r\n"
        "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
        "Connection: Close\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html>\r\n"
        "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
        "</html>\r\n";

void test_response() {
    seaice::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    SEAICE_LOG_DEBUG(logger) << " s = " << s
        <<" has error = " << parser.hasError()
        <<" is finish = " << parser.isFinish()
        <<" total size = " << tmp.size()
        <<" Content-Length = " << parser.getContentLength();
    SEAICE_LOG_DEBUG(logger) << parser.getData()->toString();
    tmp.resize(tmp.size() - s);
    SEAICE_LOG_DEBUG(logger) << tmp;
}

int main () {
    test_req();
    test_res();
    test_http_parser();
    test_response();
    return 0;
}