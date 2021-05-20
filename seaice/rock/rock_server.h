#ifndef __SEAICE_ROCK_SERVER_H__
#define __SEAICE_ROCK_SERVER_H__

#include "../TcpServer.h"


namespace seaice {

class RockServer : public TcpServer
{
public:
    RockServer() {}
    ~RockServer() {}

    virtual void handleClient(Socket::ptr sock) override;
};


}

#endif