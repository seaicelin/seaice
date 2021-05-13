#include "test.h"
#include "../seaice/ds/timed_cache.h"

#define SIZE 105

static seaice::Logger::ptr logger = SEAICE_LOGGER("system");

void test_time_cache() {
    seaice::ds::TimeCache<int, int> cache(130, 10);
    for(int i = 0; i < SIZE; i++) {
        cache.set(i, i * 100, 1000);
    }

    SEAICE_LOG_DEBUG(logger) << "hello world";

    for(int i = 0; i < SIZE; i++) {
        int v;
        if (cache.get(i, v)) {
            std::cout << "get: " << i << " - "
                      << v
                      << " - " << cache.get(i)
                      << std::endl;
        }
    }

    cache.set(1000, 11, 1000 * 10);
    std::cout << cache.toStatusString() << std::endl;
    sleep(2);
    std::cout << "check_timeout: " << cache.checkTimeout() << std::endl;
    std::cout << cache.toStatusString() << std::endl;
}

void test_hash_timed_cache() {
    seaice::ds::HashTimeCache<int, int> cache(4, 30, 10);

    for(int i = 0; i < SIZE; i++) {
        cache.set(i, i* 100, -1000 * i);
    }

    for(int i = 0; i < SIZE; i++) {
        int v;
        if (cache.get(i, v)) {
            std::cout << "get: " << i << " - "
                      << v
                      << " - " << cache.get(i)
                      << std::endl;
        }
    }

    cache.expired(100, 1000 * 10);
    cache.set(1000, 11, 1000 * 10);
    std::cout << "expired: " << cache.expired(100, 1000 * 10) << std::endl;
    std::cout << cache.toStatusString() << std::endl;
    sleep(2);
    std::cout << "check_timeout: " << cache.checkTimeout() << std::endl;
    std::cout << cache.toStatusString() << std::endl;
}


int main(char* argc, char** argv){
    //test_time_cache();
    test_hash_timed_cache();
    return 0;
}