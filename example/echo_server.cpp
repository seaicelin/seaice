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
    SEAICE_LOG_DEBUG(logger) << "handleClient" << *sock;
	seaice::ByteArray::ptr ba(new seaice::ByteArray); 
	std::vector<iovec> iovecs;
	const int SIZE = 1024;
	sock->setRecvTimeout(2000);
	while(true) {
        SEAICE_LOG_DEBUG(logger) << "sock recv";
        ba->clear();
        iovecs.clear();
        int len = ba->getWriteBuffers(iovecs, SIZE);
        int rt = sock->recv(&iovecs[0], len, 0);
        SEAICE_LOG_DEBUG(logger) << "receive rt = " << rt;

        if(rt == 0) {
            break;
        } else if (rt == -1) {
            continue;
        }

        //设置读取到的数据的下标
        ba->setSize(ba->getPos() + rt);
        iovecs.clear();

        len = ba->getReadBuffers(iovecs, rt);
        rt = sock->send(&iovecs[0], len, 0);
        SEAICE_LOG_DEBUG(logger) << "send rt = " << rt;

        SEAICE_LOG_DEBUG(logger) << "receive msg = " << ba->toString();
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

