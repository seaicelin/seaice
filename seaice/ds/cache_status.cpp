#include "cache_status.h"
#include "../utils.h"
#include <sstream>

namespace seaice {
namespace ds {

using namespace seaice::utils;

CacheStatus::CacheStatus() 
    : m_set(0)
    , m_get(0)
    , m_del(0)
    , m_timeout(0)
    , m_prune(0)
    , m_hit(0) {
}

int64_t CacheStatus::incGet(int64_t v) {
    return Atomic::addFetch(m_get, v);
}

int64_t CacheStatus::incSet(int64_t v) {
    return Atomic::addFetch(m_set, v);
}

int64_t CacheStatus::incDel(int64_t v) {
    return Atomic::addFetch(m_del, v);
}

int64_t CacheStatus::incTimeout(int64_t v) {
    return Atomic::addFetch(m_timeout, v);
}

int64_t CacheStatus::incPrune(int64_t v) {
    return Atomic::addFetch(m_prune, v);
}

int64_t CacheStatus::incHit(int64_t v) {
    return Atomic::addFetch(m_hit, v);
}

int64_t CacheStatus::delGet(int64_t v) {
    return Atomic::subFetch(m_get, v);
}

int64_t CacheStatus::delSet(int64_t v) {
    return Atomic::subFetch(m_set, v);
}

int64_t CacheStatus::delDel(int64_t v) {
    return Atomic::subFetch(m_del, v);
}

int64_t CacheStatus::delTimeout(int64_t v) {
    return Atomic::subFetch(m_timeout, v);
}

int64_t CacheStatus::delPrune(int64_t v) {
    return Atomic::subFetch(m_prune, v);
}

int64_t CacheStatus::delHit(int64_t v) {
    return Atomic::subFetch(m_hit, v);
}

int64_t CacheStatus::getGet() const {
    return m_get;
}

int64_t CacheStatus::getSet() const {
    return m_set;
}

int64_t CacheStatus::getDel() const {
    return m_del;
}

int64_t CacheStatus::getTimeout() const {
    return m_timeout;
}

int64_t CacheStatus::getPrune() const {
    return m_prune;
}

int64_t CacheStatus::getHit() const {
    return m_hit;
}

double CacheStatus::getHitRate() const {
    return m_get ? (m_hit * 1.0 / m_get) : 0;
}

void CacheStatus::merge(const CacheStatus& c) {
    m_get += c.m_get;
    m_set += c.m_set;
    m_del += c.m_del;
    m_timeout += c.m_timeout;
    m_prune += c.m_prune;
    m_hit += c.m_hit;
}

std::string CacheStatus::toString() const {
    std::stringstream ss;
    ss  << "[get = " << m_get
        << " set = " << m_set
        << " del = " << m_del
        << " timeout = " << m_timeout
        << " prune = " << m_prune
        << " hit = " << m_hit
        << " hit rate = " << getHitRate() * 100 << "%"
        <<"]";
    return ss.str();
}


}
}