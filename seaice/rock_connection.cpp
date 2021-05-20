#include "rock_connection.h"

namespace seaice {

RockConnection::RockConnection(Socket::ptr sock
         , IOManager* worker
         , IOManager* io_worker) 
    : RockStream(sock, worker, io_worker) {
}

RockConnection::~RockConnection() {}

}