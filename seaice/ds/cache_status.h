#ifndef __SEAICE_DS_H__
#define __SEAICE_DS_H__

#include <stdint.h>
#include <string>

namespace seaice{
namespace ds{

/*
记录缓存的状态信息
1. get 的次数
2. set 的次数
3. del 的次数
4. timeout 的次数
5. hit 命中的次数
6. 需要调整的次数
*/
class CacheStatus {
public:
    CacheStatus() {}

    int64_t incGet(int64_t v = 1);
    int64_t incSet(int64_t v = 1);
    int64_t incDel(int64_t v = 1);
    int64_t incTimeout(int64_t v = 1);
    int64_t incPrune(int64_t v = 1);
    int64_t incHit(int64_t v = 1);

    int64_t delGet(int64_t v = 1);
    int64_t delSet(int64_t v = 1);
    int64_t delDel(int64_t v = 1);
    int64_t delTimeout(int64_t v = 1);
    int64_t delPrune(int64_t v = 1);
    int64_t delHit(int64_t v = 1);

    int64_t getGet() const;
    int64_t getSet() const;
    int64_t getDel() const;
    int64_t getTimeout() const;
    int64_t getPrune() const;
    int64_t getHit() const;

    double getHitRate() const;
    void merge(const CacheStatus& c);
    std::string toString() const;

private:
    int64_t m_set;
    int64_t m_get;
    int64_t m_del;
    int64_t m_timeout;
    int64_t m_prune;
    int64_t m_hit;
};


}
}

#endif