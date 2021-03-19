#ifndef __SEAICE_FD_MANAGER_H__
#define __SEAICE_FD_MANAGER_H__

#include <memory>
#include <stdint.h>
#include <vector>
#include "mutex.h"
#include "singleton.h"
#include "thread.h"

namespace seaice {

//只需要 hook soket 相关的操作函数
//所以需要通过文件句柄属性来辨别
//当前操作的文件描述符是否 socket
//如果不是socket, 不需要 hook 函数

class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;
    FdCtx(int fd);
    ~FdCtx();

    bool init();
    bool isInit() const {return m_isInit;}
    bool isSocket() const {return m_isSocket;}
    bool isSysNonBlock() const {return m_sysNonBlock;}
    bool isUserNonBlock() const {return m_userNonBlock;}
    bool isClose() const {return m_isClosed;}

    void setUserNonBlock(bool v) {m_userNonBlock = v;}
    bool getUserNonBlock() const {return m_userNonBlock;}

    void setSysNonBlock(bool v) {m_sysNonBlock = v;}
    bool getSysNonBlock() const {return m_sysNonBlock;}

    void setTimeout(int type, uint64_t v);
    uint64_t getTimeout(int type);

private:
    int m_fd;
    bool m_isInit;
    bool m_isSocket;
    bool m_sysNonBlock;
    bool m_userNonBlock;
    bool m_isClosed;
    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;
};


class FdManager {
public:
    typedef RWMutex MutexType;
    FdManager();

    FdCtx::ptr get(int fd, bool auto_create = false);
    void del(int fd);

private:
    MutexType m_mutex;
    std::vector<FdCtx::ptr> m_datas;
};

typedef Singleton<FdManager> FdMgr;

}


#endif