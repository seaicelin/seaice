#ifndef _SEAICE_UTILS_H_
#define _SEAICE_UTILS_H_

#include <string>
#include <execinfo.h>
#include <typeinfo>
#include <vector>
#include <boost/lexical_cast.hpp>

namespace seaice {
namespace utils {

long int getThreadId();

int createFileDir(const std::string& path);

int IsFileExist(const char* path);

int IsDirExist(const char* path);

std::string print_backtrace();

uint64_t getCurrentMs();

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

class FSUtil {
public:
    static void ListAllFile(std::vector<std::string>& files
                          , const std::string& path
                          , const std::string& subfix);
    static bool Mkdir(const std::string& dirname);
    static bool IsRunningPidfile(const std::string& pidfile);
};

template<typename T>
std::string TypeToName() {
    return std::string(typeid(T).name());
}

template<class Map, class K, class V>
V getParamValue(const Map& m, const K& k, const V& def = V()) {
    auto it = m.find(k);
    if(it == m.end()) {
        return def;
    }
    try {
        return boost::lexical_cast<V>(it->second);
    } catch (...) {
    }
    return def;
}

template<class Map, class K, class V>
bool checkGetParamValue(const Map& m, const K& k, V& v) {
    auto it = m.find(k);
    if(it == m.end()) {
        return false;
    }
    try {
        v = boost::lexical_cast<V>(it->second);
        return true;
    } catch(...){
    }
    return false;
}

/*
[20210507]多綫程無鎖編程_sync_fetch_and_add.txt
*/
class Atomic {
public:
    template<typename T, typename S>
    static T addFetch(volatile T& t, S v = 1) {
        return __sync_add_and_fetch(&t, (T)v);
    }

    template<typename T, typename S>
    static T subFetch(volatile T& t, S v = 1) {
        return __sync_sub_and_fetch(&t, (T)v);
    }

    template<typename T, typename S>
    static T orFetch(volatile T& t, S v) {
        return __sync_or_and_fetch(&t, (T)v);
    }

    template<typename T, typename S>
    static T andFetch(volatile T& t, S v) {
        return __sync_and_and_fetch(&t, (T)v);
    }

    template<typename T, typename S>
    static T xorFetch(volatile T& t, S v) {
        return __sync_xor_and_fetch(&t, (T)v);
    }

    template<typename T, typename S>
    static T nandFetch(volatile T& t, S v) {
        return __sync_nand_and_fetch(&t, (T)v);
    }

    template<typename T, typename S>
    static T fetchAdd(volatile T& t, S v = 1) {
        return __sync_fetch_and_add(&t, (T)v);
    }

    template<typename T, typename S>
    static T fetchSub(volatile T& t, S v = 1) {
        return __sync_fetch_and_sub(&t, (T)v);
    }

    template<typename T, typename S>
    static T fetchOr(volatile T& t, S v) {
        return __sync_fetch_and_or(&t, (T)v);
    }

    template<typename T, typename S>
    static T fetchAnd(volatile T& t, S v) {
        return __sync_fetch_and_and(&t, (T)v);
    }

    template<typename T, typename S>
    static T fetchXor(volatile T& t, S v) {
        return __sync_fetch_and_xor(&t, (T)v);
    }

    template<typename T, typename S>
    static T fetchNand(volatile T& t, S v) {
        return __sync_fetch_and_nand(&t, (T)v);
    }

    template<typename T, typename S>
    static T compareAndSwap(volatile T& t, S old_value, S new_value) {
        return __sync_val_compare_and_swap(&t, (T)old_value, (T)new_value);
    }

    template<typename T, typename S>
    static bool compareAndSwapBool(volatile T& t, S old_value, S new_value) {
        return __sync_bool_compare_and_swap(&t, (T)old_value, (T)new_value);
    }

};


}
}

#endif