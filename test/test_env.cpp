#include "test.h"
#include <time.h>
#include <sys/stat.h>
#include "../seaice/daemon.h"
#include "../seaice/env.h"
#include <fstream>

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

//1. 读取环境变量
//2. 获取程序的绝对路径，基于绝对路径设置 getCwd
//3. 可以通过 cmdline,在进入 main 函数前，解析好参数

//在进入 main 函数前获取参数
struct A {
    A(){
        std::ifstream ifs("/proc/" + std::to_string(getpid()) + "/cmdline", std::ios::binary);
        std::string content;
        content.resize(1024);
        ifs.read(&content[0], content.size());
        content.resize(ifs.gcount());

        for(size_t i = 0; i < content.size(); ++i) {
            SEAICE_LOG_DEBUG(logger) << i << " - "
                << content[i] << " - "
                <<(int)content[i];
        }
    }
};

A a;

int main(int argc, char** argv) {
    std::cout<< "argc = " << argc << std::endl;
    seaice::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    seaice::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    seaice::EnvMgr::GetInstance()->addHelp("p", "print help");

    if(!seaice::EnvMgr::GetInstance()->init(argc, argv)) {
        seaice::EnvMgr::GetInstance()->printHelp();
        return 0;
    }

    SEAICE_LOG_DEBUG(logger) << seaice::EnvMgr::GetInstance()->getExe();
    SEAICE_LOG_DEBUG(logger) << seaice::EnvMgr::GetInstance()->getCwd();
    SEAICE_LOG_DEBUG(logger) << seaice::EnvMgr::GetInstance()->getEnv("PATH", "XXX");

    if(seaice::EnvMgr::GetInstance()->has("p")) {
        seaice::EnvMgr::GetInstance()->printHelp();
    }
    return 0;
}