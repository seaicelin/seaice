#include "test.h"
#include <time.h>
#include <sys/stat.h>
#include "../seaice/daemon.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

#define ERR_EXIT(m) \
do { \
    perror(m); \
    exit(EXIT_FAILURE); \
} while(0);

void create_daemon(int nochdir, int noclose) {
    pid_t pid;
    pid = fork();
    if(pid == -1) {
        ERR_EXIT("fork error");
    }
    if(pid > 0) {
        exit(EXIT_SUCCESS);
    }
    if(setsid() == -1) {
        ERR_EXIT("SET SID ERROR");
    }
    if(nochdir == 0) {
        chdir("/");
    }
    if(noclose == 0) {
        int i;
        for(i = 0; i < 3; i++) {
            close(i);
            open("/dev/null", O_RDWR);
            dup(0);
            dup(0);
        }
    }
    umask(0);
}

void test() {
    time_t t;
    int fd;
    create_daemon(0, 0);
    while(1) {
        fd = open("daemon.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if(fd == -1) {
            ERR_EXIT("open error");
        }
        t = time(0);
        char* buf = asctime(localtime(&t));
        write(fd, buf, strlen(buf));
        close(fd);
        sleep(5);
    }
}


seaice::Timer::ptr timer;
int server_main(int argc, char** argv) {
    SEAICE_LOG_DEBUG(logger) << seaice::ProcessInfoMgr::GetInstance()->toString();
    seaice::IOManager iom(2);
    iom.start();
    timer = iom.addTimer(1000, [](){
        SEAICE_LOG_DEBUG(logger) << "onTimer";
        static int count = 0;
        if(++count > 10) {
            exit(1);
        }
    }, true);
    iom.stop();
}

int main(int argc, char *argv[])
{
    //test();
    //test_daemon();
    return seaice::start_daemon(argc, argv, server_main, argc != 1);
}