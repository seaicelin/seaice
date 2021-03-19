#ifndef _SEAICE_MACRO_H_
#define _SEAICE_MACRO_H_

#include <string>
#include <assert.h>
#include "log.h"
#include "utils.h"

#define SEAICE_ASSERT(x) \
     if(!(x)) { \
        SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) << "assert: " #x \
            << "\nbacktrace:\n" \
            << seaice::utils::print_backtrace(); \
        assert(x); \
    }

#define SEAICE_ASSERT2(x, w) \
     if(!(x)) { \
        SEAICE_LOG_ERROR(SEAICE_LOGGER("system")) << "assert: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << seaice::utils::print_backtrace(); \
        assert(x); \
    }

#endif