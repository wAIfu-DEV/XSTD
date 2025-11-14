#pragma once

#include "xstd_core.h"

typedef struct _time_os_int
{
    void *_internalState;

    u64 (*const timestamp_unix_ms)(void);
} _TimeOsInterface;
