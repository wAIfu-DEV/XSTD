#pragma once

#include "xstd_core.h"

typedef struct _process_os_interface
{
    void *_internalState;

    void (*const exit)(i32 code);
} _ProcOsInterface;
