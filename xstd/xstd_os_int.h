#pragma once

#include "xstd_core.h"

typedef struct _os_interface
{
    void *_internalState;

    void (* (*const signal)(i32 sigNum,  void (*handler)(i32 sigNum)))(i32 sigNum);
} _OsInterface;
