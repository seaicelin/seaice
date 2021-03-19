#include "mutex.h"

using namespace seaice;

Semaphore::Semaphore(uint32_t value) {
    sem_init(&m_sem, 0, value);
}

Semaphore::~Semaphore() {
    sem_destroy(&m_sem);
}

void Semaphore::wait() {
    sem_wait(&m_sem);
}

void Semaphore::notify() {
    sem_post(&m_sem);
}