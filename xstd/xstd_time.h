#pragma once

#include "xstd_core.h"
#include "xstd_time_os_int.h"
#include "xstd_time_os_int_default.h"

static inline u64 time_unix_ms(void)
{
    return _default_time_osint()->timestamp_unix_ms();
}
