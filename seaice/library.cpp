#include "library.h"
#include "log.h"
#include <dlfcn.h>
#include "env.h"
#include "config2.h"

/*
1. 定义创建和销毁 module 的函数指针
2. 创建释放 module 时的仿函数，用于关闭动态链接库
3. 定义 GetModule 函数，
    3.1 dlopen 打开指定路径的动态连接库
    3.2 dlsym 获取 CreateModule/DestroyModule 函数句柄
    3.3 创建 Module 对象只能指针，指定析构时的方式函数（2）
*/

namespace seaice {

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

typedef Module* (*create_module)();
typedef void (*destroy_module)(Module*);

class ModuleCloser {
public:
    ModuleCloser(void* handle, destroy_module d) 
        : m_handle(handle)
        , m_destroy(d){
    }

    void operator()(Module* m) {
        std::string name = m->getName();
        std::string version = m->getVersion();
        std::string path = m->getFilename();
        m_destroy(m);
        int rt = dlclose(m_handle);
        if(rt) {
            SEAICE_LOG_ERROR(logger) << "dlclose handle fail handle = "
                << m_handle << " name = " << name
                <<" version = " << version
                <<" path = " << path
                <<" error = " << dlerror();
        } else {
            SEAICE_LOG_INFO(logger) << "dlclose handle = "
                << m_handle << " name = " << name
                <<" version = " << version
                <<" path = " << path
                <<" SUCCESS ";
        }
    }

private:
    void* m_handle;
    destroy_module m_destroy;
};

Module::ptr Library::GetModule(const std::string& path) {
    void* handle = dlopen(path.c_str(), RTLD_NOW);
    if(!handle) {
        SEAICE_LOG_ERROR(logger) << "can not load library on path"
            << path << " error=" << dlerror();
        return nullptr;
    }

    create_module create = (create_module)dlsym(handle, "CreateModule");
    if(!create) {
        SEAICE_LOG_ERROR(logger) << "can not load symbol CreateModule in "
            << path << " error = " << dlerror();
        dlclose(handle);
        return nullptr;
    }

    destroy_module destroy = (destroy_module)dlsym(handle, "DestroyModule");
    if(!destroy) {
        SEAICE_LOG_ERROR(logger) << "can not load symbol DestroyModule in "
            << path << " error = " << dlerror();
        dlclose(handle);
        return nullptr;
    }

    //创建 module ptr 的时候指定释放函数，ModuleClose仿函数
    Module::ptr module(create(), ModuleCloser(handle, destroy));
    module->setFilename(path);
    SEAICE_LOG_INFO(logger) << "load module name = " << module->getName()
        << " version = " << module->getVersion()
        << " path = " << module->getFilename()
        << " SUCCESS";
    seaice::Config2::LoadFromConfDir(seaice::EnvMgr::GetInstance()->getConfigPath(), true);
    return module;
}



}