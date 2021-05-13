#include "test.h"

seaice::Logger::ptr logger = SEAICE_LOGGER("system");

int main(int argc, char *argv[]) {
    seaice::Application ap;
    ap.init(argc, argv);
    //std::string conf_path = seaice::EnvMgr::GetInstance()->getConfigPath();
    //seaice::Config2::LoadFromConfDir(conf_path);
    static seaice::ConfigVar<std::map<std::string, std::map<std::string, std::string> > >::ptr g_worker_config
    = seaice::Config2::Lookup("workers", 
        std::map<std::string, std::map<std::string, std::string> >(), "worker cofnfig");

    SEAICE_LOG_DEBUG(logger) << g_worker_config->toString();
    seaice::WorkMgr::GetInstance()->init(g_worker_config->getValue());
    SEAICE_LOG_DEBUG(logger) << seaice::WorkMgr::GetInstance()->toString();
    return 0;
}