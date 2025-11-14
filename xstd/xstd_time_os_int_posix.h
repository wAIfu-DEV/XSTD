#pragma once

#include "xstd_core.h"
#include "xstd_time_os_int.h"

#include "sys/time.h"
#include "time.h"

static inline u64 _time_osint_posix_unix_ms(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, 0) != 0)
        return 0;

    return (u64)tv.tv_sec * 1000uLL + (u64)(tv.tv_usec / 1000uL);
}

static const _TimeOsInterface _time_osint_posix = {
    ._internalState = NULL,
    .timestamp_unix_ms = _time_osint_posix_unix_ms,
};
