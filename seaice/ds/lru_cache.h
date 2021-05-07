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
class LruCache
{
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

	bool get(const K& k, const V& v);
	bool del(const K& k);
	bool exist(const K& k);
	size_t size();
	bool empty();
	bool clear();
	void setMaxSize(const size_t& v);
	void setElasticity(const size_t& v);
	size_t getMaxSize() const;
	size_t getElasticity() const;
	size_t getMaxAllowSize() const;
	void setPruneCallback(prune_callback cb);
	std::string toStatusString() const;
	CacheStatus* getStatus() const;
	void setStatus(CacheStatus* v, bool owner = false);

protected:
	size_t prune();

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


}
}
#endif