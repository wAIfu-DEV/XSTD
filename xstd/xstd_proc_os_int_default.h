#pragma once

// Implementation of _FileOsInterface using Cstd

#include "xstd_proc_os_int.h"

#ifdef _X_PLAT_WIN
    #include "xstd_proc_os_int_win32.h"
#else
    #include "xstd_proc_os_int_stdlib.h"
#endif

/**
 * @brief Default process os interface, interface will be switched at compile
 * time to match the compilation target.
 *
 * This function is meant to be used by XSTD.
 */
static inline const _ProcOsInterface* _default_proc_os_int(void) {
    #ifdef _X_PLAT_WIN
        return &_proc_os_int_win32;
    #else
        return &_proc_os_int_stdlib;
    #endif
}
