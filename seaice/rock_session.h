#ifndef __SEAICE_ROCK_SESSION_H__
#define __SEAICE_ROCK_SESSION_H__

#include "rockstream.h"

namespace seaice {

class RockSession : public RockStream {
public:
    RockSession(Socket::ptr sock
             , IOManager* worker = IOManager::GetThis()
             , IOManager* io_worker = IOManager::GetThis());

    ~RockSession();
};

}

#endif