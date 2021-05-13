#ifndef __SEAICE_TIME_CACHE_H__
#define __SEAICE_TIME_CACHE_H__

#include <stdint.h>
#include <memory>
#include <set>
#include <vector>
#include <functional>
#include <iostream>
#include <unordered_map>
#include "cache_status.h"
#include "../utils.h"

namespace seaice{
namespace ds{
/*
1. 定义一个节点结构 Item，存储超时时间，键，值.
2. 定义一个有序集合 set 存储 Item.最早超时节点放在最前面
3. 定义 unorder_map 存放键，集合迭代器。
*/
template<typename K, typename V, typename RWMutexType = seaice::RWMutex>
class TimeCache {
private:
    struct Item {
        Item(const K& k, const V& v, const uint64_t& ms)
            : key(k)
            , val(v)
            , ts(ms) {
        }
        K key;
        mutable V val;
        uint64_t ts;

        bool operator< (const Item& rhs) const {
            if(ts != rhs.ts) {
                return ts < rhs.ts;
            }
            return key < rhs.key;
        }

        std::string toString() const {
            std::stringstream ss;
            ss << "[key = " << key
               << ", val = " << val
               << ", ts = " << ts
               << "]";
            return ss.str();
        }
    };
public:
    typedef std::shared_ptr<TimeCache> ptr;
    typedef Item item_type;
    typedef std::set<item_type> set_type;
    typedef std::unordered_map<K, typename set_type::iterator> map_type;
    typedef std::function<void(const K& k, const V& v)> prune_callback;

    TimeCache(size_t max_size = 0, size_t elasticity = 0
            , CacheStatus* status = nullptr) 
        : m_maxSize(max_size) 
        , m_elasticity(elasticity)
        , m_status(status) {
        if(m_status == nullptr) {
            m_status = new CacheStatus;
            m_statusOnwer = true;
        }
    }

    ~TimeCache() {
        if(m_status && m_statusOnwer) {
            delete m_status;
        }
    }

    void set(const K& k, const V& v, uint64_t timeout_ms) {
        m_status->incSet();
        typename RWMutexType::WriteLock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it != m_cache.end()) {
            m_cache.erase(it);
        }
        auto sit = m_timed.insert(Item(k, v, timeout_ms + seaice::utils::getCurrentMs()));
        m_cache.insert(std::make_pair(k, sit.first));
        //std::cout << "set : k = " << k << " v = " << v << std::endl;
        //std::cout << "set: " << sit.first->toString() << std::endl;
        prune();
    }

    bool get(const K& k, V& v) {
        m_status->incGet();
        typename RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it == m_cache.end()) {
            lock.unlock();
            return false;
        }
        //std::cout << " get: " << (it->second)->toString() << std::endl;
        v = it->second->val;
        lock.unlock();
        m_status->incHit();
        return true;
    }

    V get(const K& k) {
        m_status->incGet();
        typename RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it == m_cache.end()) {
            lock.unlock();
            return V();
        }
        auto v = it->second->val;
        lock.unlock();
        m_status->incHit();
        return v;
    }

    bool del(const K& k) {
        m_status->incDel();
        typename RWMutexType::WriteLock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it == m_cache.end()) {
            return false;
        }
        m_timed.erase(it->second);
        m_cache.erase(it);
        lock.unlock();
        m_status->incHit();
    }

    bool expired(const K& k, const uint64_t& ts) {
        typename RWMutexType::WriteLock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it == m_cache.end()) {
            return false;
        }
        uint64_t tts = ts + seaice::utils::getCurrentMs();
        if(tts == it->second->ts) {
            return true;
        }
        auto item = *it->second;
        m_timed.erase(it->second);
        auto iit = m_timed.insert(item);
        it->second = iit.first;
        return true;
    }

    bool exist(const K& k) {
        typename RWMutexType::ReadLock lock(m_mutex);
        return m_cache.find(k) != m_cache.end();
    }

    size_t size() {
        typename RWMutexType::ReadLock lock(m_mutex);
        return m_cache.size();
    }

    bool empty() {
        typename RWMutexType::ReadLock lock(m_mutex);
        return m_cache.empty();
    }

    bool clear() {
        typename RWMutexType::WriteLock lock(m_mutex);
        m_timed.clear();
        m_cache.clear();
        return true;
    }

    void setMaxSize(const size_t& v) {
        m_maxSize = v;
    }

    void setElasticity(const size_t& v) {
        m_elasticity = v;
    }

    size_t getMaxSize() const {
        return m_maxSize;
    }

    size_t getElasticity() const {
        return m_elasticity;
    }

    size_t getMaxAllowedSize() const {
        return m_maxSize + m_elasticity;
    }

    template<typename F>
    void foreach(F& f) {
        typename RWMutexType::ReadLock lock(m_mutex);
        std::for_each(m_cache.begin(), m_cache.end(), f);
    }

    void setPruneCallback(prune_callback cb) {
        m_cb = cb;
    }

    std::string toStatusString() {
        std::stringstream ss;
        ss << (m_status? m_status->toString() : std::string("(no status)"));
        ss << " total = " << size();
        return ss.str();
    }

    CacheStatus* getStatus() const {
        return m_status;
    }

    void setStatus(CacheStatus* v, bool owner = false) {
        if(m_statusOnwer && m_status) {
            delete m_status;
        }
        m_status = v;
        m_statusOnwer = owner;
        if(m_status == nullptr) {
            m_status = new CacheStatus;
            m_statusOnwer = true;
        }
    }

    size_t checkTimeout(const uint64_t& ts = seaice::utils::getCurrentMs()) {
        size_t sz = 0;
        typename RWMutexType::WriteLock lock(m_mutex);
        for(auto it = m_timed.begin();
                it != m_timed.end();) {
            if(it->ts <= ts) {
                if(m_cb) {
                    m_cb(it->key, it->val);
                }
                m_cache.erase(it->key);
                m_timed.erase(it++);
                ++sz;
            } else {
                break;
            }
        }
        return sz;
    }

protected:
    size_t prune() {
        if(m_maxSize == 0 || m_cache.size() < getMaxAllowedSize()) {
            return 0;
        }
        size_t count = 0;
        while(m_cache.size() > m_maxSize) {
            auto it = m_timed.begin();
            if(m_cb) {
                m_cb(it->key, it->val);
            }
            m_cache.erase(it->key);
            m_timed.erase(it);
            ++count;
        }
        m_status->incPrune();
        return count;
    }

private:
    size_t m_maxSize;
    size_t m_elasticity;
    CacheStatus* m_status;
    RWMutexType m_mutex;
    map_type m_cache;
    set_type m_timed;
    prune_callback m_cb;
    bool m_statusOnwer = false;
};


template<typename K, typename V, typename RWMutexType = seaice::RWMutex,
        typename Hash = std::hash<K>>
class HashTimeCache {
public:
    typedef std::shared_ptr<HashTimeCache> ptr;
    typedef TimeCache<K, V, RWMutexType> cache_type;

    HashTimeCache(size_t bucket, size_t max_size, size_t elasticity)
        : m_bucket(bucket) {
        m_datas.resize(bucket);
        size_t pre_max_size = std::ceil(max_size * 1.0 / bucket);
        size_t pre_elasticity = std::ceil(elasticity * 1.0 / bucket);

        m_maxSize = pre_max_size * bucket;
        m_elasticity = pre_elasticity * bucket;

        for(size_t i = 0; i < bucket; i++) {
            m_datas[i] = new cache_type(pre_max_size, pre_elasticity, &m_status);
        }
    }

    ~HashTimeCache() {
        for(size_t i = 0; i < m_bucket; i++) {
            delete m_datas[i];
        }
    }

    void set(const K& k, const V& v, uint64_t ms) {
        m_datas[m_hash(k) % m_bucket]->set(k, v, ms);
    }

    bool expired(const K& k, const uint64_t& ms) {
        return m_datas[m_hash(k) % m_bucket]->expired(k, ms);
    }

    bool get(const K& k, V& v) {
        return m_datas[m_hash(k) % m_bucket]->get(k, v);
    }

    V get(const K& k) {
        return m_datas[m_hash(k) % m_bucket]->get(k);
    }

    bool del(const K& k) {
        return m_datas[m_hash(k) % m_bucket]->del(k);
    }

    bool exist(const K& k) {
        return m_datas[m_hash(k) % m_bucket]->exist(k);
    }

    size_t size() {
        size_t total = 0;
        for(auto& i : m_datas) {
            total += i->size();
        }
        return total;
    }

    bool empty() {
        for(auto& i : m_datas) {
            if(!i->empty()) {
                return false;
            }
        }
        return true;
    }

    void clear() {
        for(auto& i : m_datas) {
            i->clear();
        }
    }

    size_t getMaxSize() const {
        return m_maxSize;
    }

    size_t getElasticity() const {
        return m_elasticity;
    }

    size_t getMaxAllowedSize() const {
        return m_maxSize + m_elasticity;
    }

    void setMaxSize(const size_t& v) {
        size_t pre_max_size = std::ceil(v * 1.0 / m_bucket);
        m_maxSize = pre_max_size * m_bucket;
        for(auto& i : m_datas) {
            i->setMaxSize(pre_max_size);
        }
    }

    void setElasticity(const size_t& v) {
        size_t pre_elasticity = std::ceil(v * 1.0 / m_bucket);
        m_elasticity = pre_elasticity * m_bucket;
        for(auto& i : m_datas) {
            i->setElasticity(pre_elasticity);
        }
    }

    template<typename F>
    void foreach(F& f) {
        for(auto& i : m_datas) {
            i->foreach(f);
        }
    }

    void setPruneCallback(typename cache_type::prune_callback cb) {
        for(auto& i : m_datas) {
            i->setPruneCallback(cb);
        }
    }

    CacheStatus* getStatus() {
        return &m_status;
    }

    std::string toStatusString() {
        std::stringstream ss;
        ss << m_status.toString() << " total = " << size();
        return ss.str();
    }

    size_t checkTimeout(const uint64_t& ts = seaice::utils::getCurrentMs()) {
        size_t sz = 0;
        for(auto& i : m_datas) {
            sz += i->checkTimeout(ts);
        }
        return sz;
    }

private:
    size_t m_bucket;
    size_t m_maxSize;
    size_t m_elasticity;
    Hash m_hash;
    CacheStatus m_status;
    std::vector<cache_type*> m_datas;
};

}
}

#endif