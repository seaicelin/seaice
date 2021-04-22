#include "application.h"
#include "log.h"
#include "env.h"
#include "daemon.h"

namespace seaice{

static Logger::ptr logger = SEAICE_LOGGER("system");

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

bool Application::init(int argc, char** argv) {
    m_argc = argc;
    m_argv = argv;

    seaice::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    seaice::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    seaice::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    seaice::EnvMgr::GetInstance()->addHelp("p", "print help");

    if(!seaice::EnvMgr::GetInstance()->init(argc, argv)) {
        seaice::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    if(seaice::EnvMgr::GetInstance()->has("p")) {
        seaice::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    int run_type = 0;
    if(seaice::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if(seaice::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    if(run_type == 0) {
        seaice::EnvMgr::GetInstance()->printHelp();
        return false;
    }



    return true;
}

void Application::run() {
    bool is_daemon = seaice::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
            std::bind(&Application::main, this, std::placeholders::_1,
                std::placeholders::_2), is_daemon);
}

int Application::main(int argc, char** argv);

}