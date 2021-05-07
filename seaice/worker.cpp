#include "worker.h"
#include "config2.h"
#include "utils.h"

namespace seaice {

static ConfigVar<std::map<std::string, std::map<std::string, std::string> > >::ptr g_worker_config
    = Config2::Lookup("workers", std::map<std::string, std::map<std::string, std::string> >(), "worker cofnfig");

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

WorkerGroup::ptr WorkerGroup::Create(uint32_t batch_size, 
                                seaice::Scheduler* s) {
    return std::make_shared<WorkerGroup>(batch_size, s);
}


WorkerGroup::WorkerGroup(uint32_t batch_size, seaice::Scheduler* s) 
    : m_batchSize(batch_size)
    , m_sch(s)
    , m_finish(false)
    , m_sem(batch_size) {
}

WorkerGroup::~WorkerGroup() {
    waitAll();
}

void WorkerGroup::shcedule(std::function<void()> cb, int thread) {
    m_sem.wait();
    m_sch->schedule(std::bind(&WorkerGroup::doWork, shared_from_this()
                            , cb), thread);
}

void WorkerGroup::waitAll() {
    if(!m_finish) {
        m_finish = true;
        for(uint32_t i = 0; i < m_batchSize; i++) {
            m_sem.wait();
        }
    }
}

void WorkerGroup::doWork(std::function<void()> cb) {
    cb();
    m_sem.notify();
}

WorkerManager::WorkerManager()
    : m_stop(false) {
}

void WorkerManager::add(Scheduler::ptr s) {
    m_datas[s->getName()].push_back(s);
}

Scheduler::ptr WorkerManager::get(const std::string name) {
    auto it = m_datas.find(name);
    if(it == m_datas.end()) {
        return nullptr;
    }
    if(it->second.size() == 1) {
        return it->second[0];
    }
    return it->second[rand() % it->second.size()];
}

IOManager::ptr WorkerManager::getAsIOManager(const std::string& name) {
    return std::dynamic_pointer_cast<IOManager>(get(name));
}

bool WorkerManager::init() {
    auto workers = g_worker_config->getValue();
    SEAICE_LOG_DEBUG(logger) << g_worker_config->toString();
    return init(workers);
}

bool WorkerManager::init(const std::map<std::string, std::map<std::string, std::string> >& v) {
    for(auto& it : v) {
        std::string name = it.first;
        int32_t thread_num = seaice::utils::getParamValue(it.second, "thread_num", 1);
        int32_t worker_num = seaice::utils::getParamValue(it.second, "worker_num", 1);
        SEAICE_LOG_DEBUG(logger) << "work manager create name =" << name
            << " thread_num = " << thread_num << " worker_num = " << worker_num;
        for(int32_t i = 0; i < worker_num; ++i) {
            Scheduler::ptr s;
            if(!worker_num) {
                s = std::make_shared<IOManager>(thread_num, name);
            } else {
                s = std::make_shared<IOManager>(thread_num, name + "_" + std::to_string(i));
            }
            add(s);
        }
    }
    m_stop = m_datas.empty();
    return true;
}
void WorkerManager::stop() {
    if(m_stop) {
        return;
    }
    for(auto& it : m_datas) {
        for(auto& n : it.second) {
            n->schedule([](){});
            n->stop();
        }
    }
    m_datas.clear();
    m_stop = true;
}

bool WorkerManager::isStoped() {
    return m_stop;
}

std::ostream& WorkerManager::dump(std::ostream& os) const{
    for(auto& it : m_datas) {
        for(auto& n : it.second) {
            n->dump(os) << std::endl;
        }
    }
    return os;
}

std::string WorkerManager::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

uint32_t WorkerManager::getCount() {
    return m_datas.size();
}



}