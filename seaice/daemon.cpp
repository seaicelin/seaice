#include "daemon.h"
#include "log.h"
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

namespace seaice{

static Logger::ptr logger = SEAICE_LOGGER("system");

std::string ProcessInfo::toString() const {
    std::stringstream ss;
    ss << "[ProcessInfo parent_id = " << parent_id
       << " main_id = " << main_id
       << " parent_start_time = " << seaice::utils::Time2Str(parent_start_time)
       << " main_start_time = " << seaice::utils::Time2Str(main_start_time)
       << " restart_count = " << restart_count << "]";
    return ss.str();
}

static int real_start(int argc, char** argv
                      , std::function<int(int argc, char** argv)> main_cb) {
    ProcessInfoMgr::GetInstance()->main_id = getpid();
    ProcessInfoMgr::GetInstance()->main_start_time = time(0);
    return main_cb(argc, argv);
}

static int real_daemon(int argc, char** argv
                      , std::function<int(int argc, char** argv)> main_cb) {
    ProcessInfoMgr::GetInstance()->parent_id = getpid();
    ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
    while(true) {
        pid_t pid = fork();
        if(pid == 0) {
            //ProcessInfoMgr::GetInstance()->main_id = getpid();
            //ProcessInfoMgr::GetInstance()->main_start_time = time(0);
            SEAICE_LOG_DEBUG(logger) << "process start pid = " << getpid();
            return real_start(argc, argv, main_cb);
        } else if(pid < 0) {
            SEAICE_LOG_ERROR(logger) << "fork fail return = " << pid
                <<" errno = " << errno << " errstr = " << strerror(errno);
            return -1;
        } else {
            int status = 0;
            waitpid(pid, &status, 0);
            if(status) {
                if(status == 9) {
                    SEAICE_LOG_DEBUG(logger) << "killed";
                    break;
                } else {
                    SEAICE_LOG_ERROR(logger) << "child crash pid = " << pid
                        << " status = " << status;
                }
            } else {
                SEAICE_LOG_DEBUG(logger) << "child crash pid = " << pid;
                break;
            }
            ProcessInfoMgr::GetInstance()->restart_count++;
            sleep(5);
        }
    }
    return 0;
}

int start_daemon(int argc, char** argv
                 , std::function<int(int argc, char** argv)> main_cb
                 , bool is_daemon) {
    if(!is_daemon) {
        ProcessInfoMgr::GetInstance()->parent_id = getpid();
        ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
        return real_start(argc, argv, main_cb);
    }
    return real_daemon(argc, argv, main_cb);

}

}