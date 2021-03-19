#include <atomic>
#include <stdint.h>
#include "macro.h"
#include "thread.h"
#include "fiber.h"
#include "log.h"

namespace seaice {

static thread_local Fiber::ptr t_main_fiber = nullptr;
static thread_local Fiber::ptr t_cur_fiber = nullptr;

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static Logger::ptr logger = SEAICE_LOGGER("system");

Fiber::Fiber()
    : m_id(0) {
    m_state = EXEC;
    ++s_fiber_count;
    if(getcontext(&m_ctx)) {
        SEAICE_ASSERT2(false, "getcontext failed");
    }
    SEAICE_LOG_DEBUG(logger) << "Fiber() id = " << m_id;
}

Fiber::Fiber(Callback cb, uint32_t size)
    : m_id(++s_fiber_id) 
    , m_size(size)
    , m_cb(cb) {

    ++s_fiber_count;
    m_state = INIT;
    if(getcontext(&m_ctx)) {
        SEAICE_ASSERT2(false, "getcontext failed");
    }
//    Fiber::ptr main_fiber = Fiber::getMainFiber();
    m_stack = (char*)StackMalloc::allocaStack(size);
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_size;
    m_ctx.uc_link = nullptr;//&main_fiber->m_ctx;
    makecontext(&m_ctx, &Fiber::run, 0);
    SEAICE_LOG_DEBUG(logger) << "Fiber() id = " << m_id;
}

Fiber::~Fiber() {

    SEAICE_LOG_DEBUG(logger) << "~Fiber() id = " << m_id
        << " state = " << m_state;

    if(m_id == 0) {
        return;
    }

    SEAICE_ASSERT(m_state == TERM ||
        m_state == EXECPT || m_state == INIT);
    if(m_stack) {
        StackMalloc::deallocStack(m_stack, m_size);
    }
}

void Fiber::swapIn() {
    Fiber::ptr main_fiber = Fiber::getMainFiber();
    SEAICE_ASSERT(main_fiber);
    SEAICE_ASSERT(main_fiber->m_state == EXEC);
    SEAICE_ASSERT(m_state == HOLD || m_state == INIT
                    || m_state == READY);

    main_fiber->m_state = HOLD;
    m_state = EXEC;
    Fiber::ptr cur = shared_from_this();
    setThis(cur);
    swapcontext(&main_fiber->m_ctx, &this->m_ctx);
}

void Fiber::swapOut() {
    Fiber::ptr main_fiber = Fiber::getMainFiber();
    SEAICE_ASSERT(main_fiber);
    SEAICE_ASSERT(main_fiber->m_state == HOLD);
    //SEAICE_ASSERT(m_state == EXEC);

    main_fiber->m_state = EXEC;
    //m_state = HOLD;
    setThis(main_fiber);
    swapcontext(&m_ctx, &main_fiber->m_ctx);
}

void Fiber::reset(std::function<void()> cb) {
    SEAICE_ASSERT(m_stack);
    SEAICE_ASSERT(m_state == TERM ||
                  m_state == EXECPT ||
                  m_state == INIT);

    if(getcontext(&m_ctx)) {
        SEAICE_ASSERT2(false, "getcontext failed");
    }

    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_size;
    m_ctx.uc_link = nullptr;
    makecontext(&m_ctx, &Fiber::run, 0);
    m_cb = cb;
    m_state = INIT;
}

void Fiber::run() {
    Fiber::ptr cur = getThis();
    Fiber::ptr main_fiber = Fiber::getMainFiber();
    SEAICE_ASSERT(main_fiber);
    SEAICE_ASSERT(cur != main_fiber);
    SEAICE_ASSERT(cur->m_state == EXEC);

    try{
        cur->m_cb();
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXECPT;
        SEAICE_LOG_ERROR(logger) << "Fiber Except: " << ex.what()
            << " fiber id = " << cur->getId()
            << std::endl
            << seaice::utils::print_backtrace();
    } catch (...) {
        cur->m_state = EXECPT;
        SEAICE_LOG_ERROR(logger) << "Fiber Except: "
            << " fiber id = " << cur->getId()
            << std::endl
            << seaice::utils::print_backtrace();
    }
    cur.reset();

    //这里重新获取主协程，可能执行到这里已经
    //切到另一个线程执行了。
    main_fiber = Fiber::getMainFiber();
    SEAICE_ASSERT(main_fiber);

    main_fiber->m_state = EXEC;
    setThis(main_fiber);
    Fiber* raw_ptr = main_fiber.get();
    main_fiber.reset();
    SEAICE_ASSERT(raw_ptr);
    setcontext(&raw_ptr->m_ctx);
}

std::shared_ptr<Fiber> Fiber::getMainFiber() {
    if (t_main_fiber == nullptr) {
        t_main_fiber = Fiber::ptr(new Fiber);
        setThis(t_main_fiber);
    }
    return t_main_fiber;
}

void Fiber::setThis(std::shared_ptr<Fiber> fiber) {
    t_cur_fiber = fiber;
}

std::shared_ptr<Fiber> Fiber::getThis() {
    return t_cur_fiber;
}

void Fiber::yieldToHold() {
    Fiber::ptr cur = Fiber::getThis();
    //SEAICE_ASSERT(cur->getState() == Fiber::EXEC);
    cur->m_state = Fiber::HOLD;
    cur->swapOut();
}

void Fiber::yieldToReady() {
    Fiber::ptr cur = Fiber::getThis();
    //SEAICE_ASSERT(cur->getState() == Fiber::EXEC);
    cur->m_state = Fiber::READY;
    cur->swapOut();
}

}
