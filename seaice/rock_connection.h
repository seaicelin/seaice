#ifndef __SEAICE_ROCK_CONNECTION_H__
#define __SEAICE_ROCK_CONNECTION_H__

#include "rockstream.h"
#include "address.h"

namespace seaice {

class RockConnection : public RockStream {
public:
    RockConnection(Socket::ptr sock
             , IOManager* worker = IOManager::GetThis()
             , IOManager* io_worker = IOManager::GetThis());
    ~RockConnection();
};

}

#endif