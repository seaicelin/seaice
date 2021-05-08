#ifndef __SEAICE_LRU_CACHE_H__
#define __SEAICE_LRU_CACHE_H__

#include <memory>
#include <functional>
#include <list>
#include <unordered_map>
#include <atomic>
#include "../mutex.h"

namespace seaice{
namespace ds{

/*
https://blog.csdn.net/zhangxiao93/article/details/52974257
1. 创建保存节点为 (K,V) 的链表，存放数据
2. 利用 unorder_map 存储数据
    1.1, K 存储键值
    1.2, V 存储一个链表的迭代器，list的迭代器不会失效
    1.3, 存储时更新链表
3. get(const K& k) 获取缓存
4. set 设置缓存，如果已经存在先删除，再存进去
5. del 删除缓存
6. exist 是否存在摸个缓存
7. size 有多少个缓存
8. empty 是否为空
9. clear 清空缓存
10. setMaxSize 设置最大缓存数量
11. setElasticity 设置可弹性增加数量
12. prune 删除过期的函数
*/
template <typename K, typename V, typename MutexType = seaice::Mutex>
class LruCache {
public:
    typedef std::shared_ptr<LruCache> ptr;

    typedef std::pair<K, V> item_type;
    typedef std::list<item_type> list_type;
    typedef std::unordered_map<K, typename list_type::iterator> map_type;
    typedef std::function<void(const K& k, const V& v)> prune_callback;

    LruCache(size_t max_size = 0, size_t elasticity = 0, CacheStatus* status = nullptr) {
        m_maxSize = max_size;
        m_elasticity = m_elasticity;
        m_status = status;
        if(!m_status) {
            m_status = new CacheStatus;
            m_statusOnwer = true;
        }
    }

    ~LruCache() {
        if(m_status && m_statusOnwer) {
            delete m_status;
        }
    }

    void set(const K& k, const V& v) {
        m_status->incSet();
        typename MutexType::Lock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it != m_cache.end()) {
            it->second->second = v;
            m_keys.splice(m_keys.begin(), m_keys, it->second);
            return;
        }
        m_keys.emplace_front(std::make_pair(k, v));
        m_cache.insert(std::make_pair(k, m_keys.begin());
        prune();
    }

    V get(const K& k) {
        m_status->incGet();
        typename MutexType::Lock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it == m_cache.end()) {
            return V();
        }
        m_keys.splice(m_keys.begin(), m_keys, it->second);
        auto v = it->second->second;
        lock.unlock();
        m_status->incHit();
        return v;
    }

    bool get(const K& k, const V& v) {
        m_status->incGet();
        typename MutexType::Lock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it == m_cache.end()) {
            return false;
        }
        m_keys.splice(m_keys.begin(), m_keys, it->second);
        v = it->second->second;
        lock.unlock();
        m_status->incHit();
        return true;
    }

    bool del(const K& k) {
        m_status->incDel();
        typename MutexType::Lock lock(m_mutex);
        auto it = m_cache.find(k);
        if(it == m_cache.end()) {
            return false;
        }
        m_keys.erase(it->second);
        m_cache.erase(it);
        return true;
    }

    bool exist(const K& k) {
        typename MutexType::Lock lock(m_mutex);
        return m_cache.find(k) != m_cache.end();
    }

    size_t size() {
        typename MutexType::Lock lock(m_mutex);
        return m_cache.size();
    }

    bool empty() {
        typename MutexType::Lock lock(m_mutex);
        return m_cache.empty();
    }

    bool clear() {
        typename MutexType::Lock lock(m_mutex);
        m_keys.clear();
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

    size_t getMaxAllowSize() const {
        return m_maxSize + m_elasticity;
    }

    void setPruneCallback(prune_callback cb) {
        m_cb = cb;
    }

    std::string toStatusString() const {
        std::stringstream ss;
        if(m_status) {
            ss << m_status->toString();
        } else {
            ss << "(no status)";
        }
        ss << "total = " << size();
        return ss.str();
    }

    CacheStatus* getStatus() const {
        return m_status;
    }

    void setStatus(CacheStatus* v, bool owner = false) {
        if(m_status && m_statusOnwer) {
            delete m_status;
        }
        m_status = v;
        m_statusOnwer = owner;
        if(m_status == nullptr) {
            m_status = new CacheStatus;
            m_statusOnwer = true;
        }
    }

    template<typename F>
    void foreach(F& f) {
        typename MutexType::Lock lock(m_mutex);
        std::for_each(m_cache.begin(), m_cache.end(), f);
    }

protected:
    size_t prune() {
        if(m_maxSize == 0 || m_cache.size() < getMaxAllowSize()) {
            return 0;
        }
        size_t count = 0;
        while(m_cache.size() > m_maxSize) {
            auto& back = m_keys.back();
            if(m_cb) {
                m_cb(back.first, back.second);
            }
            m_cache.erase(back.first);
            m_keys.pop_back();
            ++count;
        }
        m_status->incPrune();
        return count;
    }

private:
    MutexType m_mutex;
    list_type m_keys;
    map_type m_cache;
    size_t m_maxSize;
    size_t m_elasticity;
    prune_callback m_cb;
    CacheStatus* m_status = nullptr;
    bool m_statusOnwer = false;
};

template<typename K, typename V, typename MutexType = seaice::Mutex, typename Hash = std::hash<K>>
class HashLruCache {
public:
    typedef std::shared_ptr<HashLruCache> ptr;
    typedef LruCache<K, V, MutexType> cache_type;

    HashLruCache(size_t bucket, size_t max_size, size_t elasticity) {
        m_bucket(bucket);
        m_datas.resize(bucket);
        size_t pre_max_size = std::ceil(max_size * 1.0 / bucket);
        size_t pre_elasticity = std::ceil(elasticity * 1.0 / bucket);
        m_maxSize = pre_max_size * bucket;
        m_elasticity = pre_elasticity * bucket;

        for(size_t i = 0; i < bucket; i++) {
            m_datas[i] = new cache_type(pre_max_size, 
                    pre_elasticity, &m_status);
        }
    }

    ~HashLruCache() {
        for(size_t i = 0; i < bucket; i++) {
            delete m_datas[i];
        }
    }

    void set(const K& k, const V& v) {
        m_datas[m_hash(k) % bucket]->set(k, v);
    }

    bool get(const K& k, V& v) {
        return m_datas[m_hash(k) % bucket]->get(k, v);
    }

    V get(const K& k) {
        return m_datas[m_hash(k) % bucket]->get(k);
    }

    bool del(const K& k) {
        return m_datas[m_hash(k) % bucket]->del(k);
    }

    bool exist(const K& k) {
        return m_datas[m_hash(k) % bucket]->exist();
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

    size_t getMaxAllowSize() const {
        return m_maxSize + m_elasticity;
    }

    size_t getElasticity() const {
        return m_elasticity;
    }

    size_t getBucket() const {
        return m_bucket;
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

private:
    std::vector<cache_type*> m_datas;
    size_t m_maxSize;
    size_t m_bucket;
    size_t m_elasticity;
    Hash m_hash;
    CacheStatus m_status;
};

}
}
#endif