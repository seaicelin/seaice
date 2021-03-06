#ifndef _SEAICE_FIBER_H_
#define _SEAICE_FIBER_H_

#include <functional>
#include <memory>
#include <ucontext.h>


namespace seaice {

class StackMallocMemory {
public:
    static void* allocaStack(int size) {
        return malloc(size);
    }

    static void deallocStack(void* ptr, int size) {
        free(ptr);
    }
};

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;
    typedef std::function<void()> Callback;
    typedef StackMallocMemory StackMalloc;

    enum State
    {
        INIT     = 0,
        READY    = 1,
        EXEC     = 2,
        HOLD     = 3,
        TERM     = 4,
        EXECPT   = 5
    };

private:
    Fiber();

public:
    Fiber(Callback cb, uint32_t size = 1024 * 1024 *10);
    ~Fiber();

    const uint64_t getId() const {return m_id;}
    const State getState() const {return m_state;}

    void swapIn();
    void swapOut();
    void reset(std::function<void()> cb);
    std::string toStirng();
    static void run();
    static std::shared_ptr<Fiber> getMainFiber();
    static void setThis(std::shared_ptr<Fiber> fiber);
    static std::shared_ptr<Fiber> getThis();
    static void yieldToHold();
    static void yieldToReady();
    const std::string toString() const;
private:
    const std::string toStateString(State state) const;
private:
    uint64_t        m_id;
    Callback        m_cb;
    uint32_t        m_size;
    char*           m_stack;
    ucontext_t      m_ctx;
    State           m_state;
};

}
#endif