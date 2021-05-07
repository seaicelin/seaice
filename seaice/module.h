#ifndef __SEAICE_MODULE_H__
#define __SEAICE_MODULE_H__

#include <map>
#include <memory>
#include "mutex.h"
#include "stream.h"
#include "singleton.h"

/*
1. 定义 Module 对象，包括链接库名，文件路径，版本
    1.1 定义 Module 对象的状态回调函数
2. 定义 Module 管理器，添加和删除 Module 对象
    2.1 init,获取指定目录下的所有 so 文件
    2.2 通过调用 library 对象打开这些 so，创建 Module::ptr
    2.3 把 Module 对象放进 map 统一管理
*/
namespace seaice {

class Module
{
public:
    typedef std::shared_ptr<Module> ptr;
    Module(const std::string& name
            , const std::string& version
            , const std::string& filename);
    virtual ~Module();

    virtual void onBeforeArgsParse(int argc, char** argv);
    virtual void onAfterArgsParse(int argc, char** argv);

    virtual bool onLoad();
    virtual bool onUnLoad();

    virtual bool onConnect(seaice::Stream::ptr stream);
    virtual bool onDisconnect(seaice::Stream::ptr stream);

    virtual bool onServiceReady();
    virtual bool onServiceUp();

    virtual std::string statusString();

    const std::string& getName() const {return m_name;}
    const std::string& getVersion() const {return m_version;}
    const std::string& getFilename() const {return m_filename;}
    const std::string& getId() const {return m_id;}

    void setFilename(const std::string& name) {m_filename = name;}

protected:
    std::string m_name;
    std::string m_version;
    std::string m_filename;
    std::string m_id;
};


class ModuleManager
{
public:
    //typedef std::shared_ptr<ModuleManager> ptr;
    typedef RWMutex RWMutexType;
    ModuleManager();
    ~ModuleManager();

    void add(Module::ptr m);
    void del(const std::string& name);
    void delAll();
    void init();
    Module::ptr get(const std::string& name);

    void onConnect(Stream::ptr stream);
    void onDisconnect(Stream::ptr stream);
    void listAll(std::vector<Module::ptr>& v);

private:
    void initModule(const std::string& path);
private:
    RWMutexType m_mutex;
    std::map<std::string, Module::ptr> m_modules;
};

typedef Singleton<ModuleManager> ModuleMgr;

}
#endif