#ifndef __SEAICE_ROCK_CONNECTION_H__
#define __SEAICE_ROCK_CONNECTION_H__

#include "rockstream.h"
#include "../uri.h"

namespace seaice {

class RockConnection : public RockStream {
public:
    typedef std::shared_ptr<RockConnection> ptr;

    RockConnection(Socket::ptr sock
             , IOManager* worker = IOManager::GetThis()
             , IOManager* io_worker = IOManager::GetThis());
    ~RockConnection();

    static RockConnection::ptr Create(const std::string& url, uint64_t timeout_ms = -1);
};

}

#endif