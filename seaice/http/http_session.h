#ifndef __SEAICE_HTTP_SESSION_H__
#define __SEAICE_HTTP_SESSION_H__

#include "../SocketStream.h"
#include "http.h"

namespace seaice{
namespace http{

class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr sock, bool owner = true);

    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr);
};

}
}
#endif