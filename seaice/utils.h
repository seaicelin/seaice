#ifndef _SEAICE_UTILS_H_
#define _SEAICE_UTILS_H_

#include <string>
#include <execinfo.h>

namespace seaice {
namespace utils {

long int getThreadId();

int createFileDir(const std::string& path);

int IsFileExist(const char* path);

int IsDirExist(const char* path);

std::string print_backtrace();

uint64_t getCurrentMs();
}
}

#endif