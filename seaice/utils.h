#ifndef _SEAICE_UTILS_H_
#define _SEAICE_UTILS_H_

#include <string>
#include <execinfo.h>
#include <typeinfo>
#include <vector>

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

}
}

#endif