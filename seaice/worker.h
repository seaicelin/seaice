#ifndef __SEAICE_WORKER_H__
#define __SEAICE_WORKER_H__

#include "noncopyable.h"
#include "mutex.h"
#include "iomanager.h"
#include "log.h"
#include "singleton.h"
#include <map>

/*
1. 定义 WorkGroup, 用于管理指定 scheduler
    1. 每个 scheduler 可以设定同时并发的 fiber 数目
    2. 超过指定size，必须等待，调用 wait 函数，挂起当前正在执行协程
    3. 处理后，调用 notify, 让之前挂起的某个协程得到执行

2. 定义 workgroup 管理类
    2.1 解析 worker.yml 文件，创建指定的 schduler 对象
    2.2 可以指定某个 schduler 指定任务
*/
namespace seaice{

class WorkerGroup : public Noncopyable, public std::enable_shared_from_this<WorkerGroup> {
public:
    typedef std::shared_ptr<WorkerGroup> ptr;

    static WorkerGroup::ptr Create(uint32_t batch_size, 
                                    seaice::Scheduler* s = seaice::Scheduler::getThis());
    WorkerGroup(uint32_t bach_size, seaice::Scheduler* s = Scheduler::getThis());
    ~WorkerGroup();

    void shcedule(std::function<void()> cb, int thread = -1);
    void waitAll();

private:
    void doWork(std::function<void()> cb);
private:
    uint32_t        m_batchSize;
    Scheduler*      m_sch;
    bool            m_finish;
    FiberSemaphore  m_sem;
};

class WorkerManager {
public:
    WorkerManager();
    void add(Scheduler::ptr s);
    Scheduler::ptr get(const std::string name);
    IOManager::ptr getAsIOManager(const std::string& name);

    template<class FiberOrCb>
    void schedule(const std::string& name, FiberOrCb fc, int thread = -1) {
        auto s = get(name);
        if(s) {
            s->schedule(fc, thread);
        } else {
            static Logger::ptr logger = SEAICE_LOGGER("system");
            SEAICE_LOG_ERROR(logger) << "shcedule name = " << name
                << " not exist";
        }
    }

    bool init();
    bool init(const std::map<std::string, std::map<std::string, std::string> >& v);
    void stop();
    bool isStoped();
    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;
    uint32_t getCount();
private:
    std::map<std::string, std::vector<Scheduler::ptr> > m_datas;
    bool m_stop;
};

typedef Singleton<WorkerManager> WorkMgr;


}
#endif