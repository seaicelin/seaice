#include "rock_session.h"

namespace seaice {

RockSession::RockSession(Socket::ptr sock
         , IOManager* worker
         , IOManager* io_worker)
    : RockStream(sock, worker, io_worker) {
}

RockSession::~RockSession() {
}

}