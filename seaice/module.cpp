#include "module.h"
#include "config2.h"
#include "env.h"
#include "utils.h"
#include "library.h"

namespace seaice {

static ConfigVar<std::string>::ptr g_module_path
    = Config2::Lookup("module.path", std::string("module"), "module path");

Module::Module(const std::string& name
        , const std::string& version
        , const std::string& filename) 
    : m_name(name)
    , m_version(version)
    , m_filename(filename)
    , m_id(name + "/" + version) {
}

Module::~Module() {
}

void Module::onBeforeArgsParse(int argc, char** argv) {
}

void Module::onAfterArgsParse(int argc, char** argv) {
}

bool Module::onLoad() {
    return true;
}

bool Module::onUnLoad() {
    return true;
}

bool Module::onConnect(seaice::Stream::ptr stream) {
    return true;
}

bool Module::onDisconnect(seaice::Stream::ptr stream) {
    return true;
}

bool Module::onServiceReady() {
    return true;
}

bool Module::onServiceUp() {
    return true;
}

std::string Module::statusString() {
    return "";
}

ModuleManager::ModuleManager() {
}

ModuleManager::~ModuleManager() {
}

void ModuleManager::add(Module::ptr m) {
    del(m->getId());
    RWMutexType::WriteLock lock(m_mutex);
    m_modules[m->getId()] = m;
}

void ModuleManager::del(const std::string& name) {
    Module::ptr module;
    RWMutexType::WriteLock lock(m_mutex);
    auto it = m_modules.find(name);
    if(it == m_modules.end()) {
        return;
    }
    module = it->second;
    m_modules.erase(it);
    lock.unlock();
    module->onUnLoad();
}

void ModuleManager::delAll() {
    RWMutexType::ReadLock lock(m_mutex);
    auto tmp = m_modules;
    lock.unlock();
    for(auto& i : tmp) {
        del(i.first);
    }
}

void ModuleManager::init() {
    auto path = seaice::EnvMgr::GetInstance()->getAbsolutePath(g_module_path->getValue());
    std::vector<std::string> files;
    seaice::utils::FSUtil::ListAllFile(files, path, ".so");
    std::sort(files.begin(), files.end());
    for(auto& i : files) {
        initModule(i);
    }
}

void ModuleManager::initModule(const std::string& path) {
    Module::ptr m = Library::GetModule(path);
    if(m) {
        add(m);
    }
}

Module::ptr ModuleManager::get(const std::string& name) {
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_modules.find(name);
    return it == m_modules.end() ? nullptr : it->second;
}

void ModuleManager::onConnect(Stream::ptr stream) {
    std::vector<Module::ptr> ms;
    listAll(ms);
    for(auto& m : ms) {
        m->onConnect(stream);
    }
}
void ModuleManager::onDisconnect(Stream::ptr stream) {
    std::vector<Module::ptr> ms;
    listAll(ms);
    for(auto& m : ms) {
        m->onDisconnect(stream);
    }
}

void ModuleManager::listAll(std::vector<Module::ptr>& v) {
    RWMutexType::ReadLock lock(m_mutex);
    for(auto& i : m_modules) {
        v.push_back(i.second);
    }
}



}