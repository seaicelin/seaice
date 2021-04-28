#ifndef __SEAICE_APPLICATION_H__
#define __SEAICE_APPLICATION_H__

#include <vector>
#include <memory>
#include "http/http_server.h"

namespace seaice{

class Application {
public:
    Application();

    static Application* GetInstance() {return s_instance;}
    bool init(int argc, char** argv);
    int run();
private:
    int main(int argc, char** argv);
    void run_fiber();
private:
    int m_argc = 0;
    char** m_argv = nullptr;
    std::vector<seaice::http::HttpServer::ptr> m_servers;
    static Application* s_instance;
};

}
#endif