#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <cstring>
#include <dirent.h>
#include <stdint.h>
#include <time.h>
#include <vector>
#include "utils.h"
#include "log.h"

namespace seaice {
namespace utils {

using namespace std;

static __thread pid_t t_tid = -1;

static const int BACKTRACK_SIZE = 100;

long int getThreadId() {
    if(t_tid == -1){
        t_tid = static_cast<pid_t>(syscall(SYS_gettid));
    }
    return t_tid;
}

int createFileDir(const std::string& path)
{
    //0 success, -1 failed
    int ret = 0;
    if(path.empty()) {
        cout <<"path is empty, openAndCreateFile failed" << endl;
        return -1;
    }
    //默认在当前目录下创建文件
    if(path.find_first_of("/") == string::npos) {
        return 0;
    }

#define MODE (S_IRWXU | S_IRWXG | S_IRWXO)

    //int pathLen = path.size();
    int startPos = 0;
    string fileName = "";
    string folderName = "";
    if(path[0] == '/') {
        ++startPos;
    }
    string::size_type pos = path.find_first_of("/", startPos);
    while(pos != string::npos) {
        folderName = path.substr(0, pos);
        //cout <<"folderName = " << folderName << endl;
        if(!IsDirExist(folderName.c_str())) {
            int ret = mkdir(folderName.c_str(), MODE);
            if(ret != 0) {
                cout <<"createFileDir mkdir failed" << endl;
                ret = -1;
                break;
            }
        }
        startPos = pos + 1;
        pos = path.find_first_of("/", startPos);
    }
#undef MODE
    return ret;
}

//检查文件(所有类型)是否存在
//-1:存在 0:不存在
int IsFileExist(const char* path)
{
    return !access(path, F_OK);
}

//检查目录是否存在
//-1:存在 0:不存在
int IsDirExist(const char* path)
{
    DIR *dp;
    if ((dp = opendir(path)) == NULL)
    {
        return 0;
    }

    closedir(dp);
    return -1;
}

std::string print_backtrace() {
    int i,nptrs;
    void *buffers[BACKTRACK_SIZE];
    char **str;
    nptrs = backtrace(buffers,BACKTRACK_SIZE);
    str = backtrace_symbols(buffers,nptrs);
    if (str == nullptr) {
        SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) << "backtrace_symbols strings == nullptr error";
    }
    stringstream oss;
    for(i = 1; i < nptrs; ++i) {
        oss << str[i] << "\n";
        //SEAICE_LOG_DEBUG(SEAICE_LOGGER("system")) << str[i];
    }
    free(str);
    return oss.str();
}

uint64_t getCurrentMs() {
    struct timespec ts;
    int ret = ::clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    if(ret == -1) {
        SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) << "clock_gettime error";
    }
    return (ts.tv_sec * 1000 + ts.tv_nsec/1000000);
}

std::string Time2Str(time_t ts, const std::string& format) {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

time_t Str2Time(const char* str, const char* format) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    if(!strptime(str, format, &t)) {
        return 0;
    }
    return mktime(&t);
}

void FSUtil::ListAllFile(std::vector<std::string>& files
                      , const std::string& path
                      , const std::string& subfix) {

    if(access(path.c_str(), 0) != 0) {
        SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) << "ListAllFile access failed"
            << " file path = " << path;
        return;
    }
    DIR* dir = opendir(path.c_str());
    if(dir == nullptr) {
        SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) << "ListAllFile opendir failed"
                    << " file path = " << path;
        return;
    }
    struct dirent* dp = nullptr;
    while((dp = readdir(dir)) != nullptr) {
        if(dp->d_type == DT_DIR) {
            if(!strcmp(dp->d_name, ".")
                || !strcmp(dp->d_name, "..")) {
                continue;
            }
            ListAllFile(files, path + "/" + dp->d_name, subfix);
        } else if(dp->d_type == DT_REG) {
            std::string filename(dp->d_name);
            if(subfix.empty()) {
                files.push_back(path + "/" + filename);
            } else {
                if(filename.size() < subfix.size()) {
                    continue;
                }
                if(filename.substr(filename.length() - subfix.size()) == subfix) {
                    files.push_back(path + "/" + filename);
                }
            }
        }
    }
    closedir(dir);
}


}//utils
}//seaice