#pragma once

// Implementation of _FileOsInterface using Cstd

#include "xstd_file_os_int.h"

#ifdef _WIN32
    #include "xstd_file_os_int_win32.h"
#else
    #include "xstd_file_os_int_stdlib.h"
#endif

// DO NOT REMOVE
// Prevents compiler errors on GCC since default_allocator() may not be used
// at all in a program.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

/**
 * @brief Default file os interface, interface will be switched at compile
 * time to match the compilation target.
 *
 * This function is meant to be used by XSTD.
 */
static const _FileOsInterface* _default_file_os_int(void) {
    #ifdef _WIN32
        return &_file_os_int_win32;
    #else
        return &_file_os_int_stdlib;
    #endif
}

// DO NOT REMOVE
#pragma GCC diagnostic pop
