#include "application.h"
#include "log.h"
#include "env.h"
#include "daemon.h"
#include "config2.h"
#include "iomanager.h"

namespace seaice{

static Logger::ptr logger = SEAICE_LOGGER("system");

static seaice::ConfigVar<std::string>::ptr g_server_work_path = 
    seaice::Config2::Lookup("server.work_path"
                            , std::string("/app/work/seaice")
                            , "server work path");

static seaice::ConfigVar<std::string>::ptr g_service_pid_file = 
    seaice::Config2::Lookup("server.pid_file"
                            , std::string("seaice.pid")
                            , "server pid file");

seaice::ConfigVar<std::vector<TcpServerConf> >::ptr g_server_config = 
    seaice::Config2::Lookup("servers"
                            , std::vector<TcpServerConf>()
                            , "http server config");

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

    std::string pidfile = g_server_work_path->getValue()
                        + "/" + g_service_pid_file->getValue();
    if(seaice::utils::FSUtil::IsRunningPidfile(pidfile)) {
        SEAICE_LOG_ERROR(logger) << "error: server is running: "
            << pidfile;
        return false;
    }

    std::string conf_path = seaice::EnvMgr::GetInstance()->getConfigPath();
    SEAICE_LOG_DEBUG(logger) << "load conf path : " << conf_path;
    seaice::Config2::LoadFromConfDir(conf_path);

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

    if(!seaice::utils::FSUtil::Mkdir(g_server_work_path->getValue())) {
        SEAICE_LOG_ERROR(logger) << "create work path: "
            << g_server_work_path->getValue() << " errno = "
            << errno << " err str = " << strerror(errno);
        return false;
    }
    SEAICE_LOG_ERROR(logger) << "application create server work path: "
        << g_server_work_path->getValue();
    return true;
}

int Application::run() {
    bool is_daemon = seaice::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
            std::bind(&Application::main, this, std::placeholders::_1,
                std::placeholders::_2), is_daemon);
}

int Application::main(int argc, char** argv){

    //把当前进程号写进文件
    std::string pidfile = g_server_work_path->getValue()
                        + "/" + g_service_pid_file->getValue();
    std::ofstream ofs(pidfile);
    if(!ofs) {
        SEAICE_LOG_ERROR(logger) << "open pifile error: " << pidfile;
        return -1;
    }
    ofs << getpid();

    seaice::IOManager iom(2, "server IOManager");
    iom.start();
    iom.schedule(std::bind(&Application::run_fiber, this));
    iom.stop();
    return 0;
}
//之前直接在main函数里面创建 socket,
//但是hook没有开，所以在 accept 卡住了
//所以要放在 iom 的schedule 里面去做
//统一放到协程框架里面去做
void Application::run_fiber() {
    //启动 服务器
    auto httpServerConfs = g_server_config->getValue();
    for(auto& conf : httpServerConfs) {
        std::vector<Address::ptr> addrVec;
        SEAICE_LOG_DEBUG(logger) << conf.toString();
        for(auto& a : conf.address) {
            if(a.find(":") == std::string::npos) {
                SEAICE_LOG_ERROR(logger) << "conf invalid address:"
                    << a;
                continue;
            }
            auto addr = seaice::Address::LookupAnyIPAddress(a);
            std::cout << *addr << std::endl;
            if(addr) {
                addrVec.push_back(addr);
            }
        }
        std::vector<Address::ptr> fails;
        seaice::http::HttpServer::ptr server(new seaice::http::HttpServer(conf.keepalive));
        server->setName(conf.name);
        server->setReadTimeout(conf.timeout);
        if(!server->bind(addrVec, fails)) {
            for(auto f : fails) {
                SEAICE_LOG_ERROR(logger) << "bind address faild at: "
                    << *f;
                _exit(0);
            }
        }
        server->start();
        m_servers.push_back(server);
    }
}

}
