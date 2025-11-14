#pragma once

#include "xstd_time_os_int.h"

#ifdef _X_PLAT_WIN
    #include "xstd_time_os_int_win32.h"
#else
    #include "xstd_time_os_int_posix.h"
#endif

static inline const _TimeOsInterface* _default_time_osint(void)
{
    #ifdef _X_PLAT_WIN
        return &_time_osint_win32;
    #else
        return &_time_osint_posix;
    #endif
}
