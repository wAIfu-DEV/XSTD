#pragma once

#include "xstd_core.h"
#include "xstd_time_os_int.h"

#include "xstd_win32.h"

static inline u64 _time_osint_win32_unix_ms(void)
{
    _w32_filetime ft;
    GetSystemTimeAsFileTime(&ft);

    const u64 epochDiff = 116444736000000000ULL;
    const u64 ticks = ((u64)ft.dwHighDateTime << 32) | (u64)ft.dwLowDateTime;

    return (ticks - epochDiff) / 10000ULL;
}

static const _TimeOsInterface _time_osint_win32 = {
    ._internalState = NULL,
    .timestamp_unix_ms = _time_osint_win32_unix_ms,
};
