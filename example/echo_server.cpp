#include "../test/test.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

using namespace seaice;

class EchoServer : public TcpServer
{
public:
	typedef std::shared_ptr<EchoServer> ptr;
	EchoServer() {}
	~EchoServer() {}
    virtual void handleClient(Socket::ptr sock); 
};

void EchoServer::handleClient(Socket::ptr sock) {
	seaice::ByteArray::ptr ba(new seaice::ByteArray); 
	std::vector<iovec> iovecs;
	const int SIZE = 1024;
	sock->setRecvTimeout(2000);
	while(true) {
		int len = ba->getWriteBuffers(iovecs, SIZE);
		int rt = sock->recv(&iovecs[0], len, 0);
		SEAICE_LOG_DEBUG(logger) << "receive rt = " << rt;
		rt = sock->send(&iovecs[0], len, 0);
		SEAICE_LOG_DEBUG(logger) << "send rt = " << rt;
		ba->setPos(0);
		SEAICE_LOG_DEBUG(logger) << "receive msg = " << ba->toString();
		ba->clear();
	}
}

void run() {
	EchoServer::ptr server(new EchoServer());
    auto addr = seaice::Address::LookupAny("[0.0.0.0:8080]");
    std::vector<seaice::Address::ptr> fails;
    std::vector<seaice::Address::ptr> addrs;
    addrs.push_back(addr);
    while(!server->bind(addrs, fails)) {
        sleep(2);
    }
    server->start();
}

int main() {
	seaice::IOManager iom(2);
	iom.start();
	iom.schedule(run);
	iom.stop();
	return 0;
}

