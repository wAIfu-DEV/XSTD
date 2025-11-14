#pragma once

#include "xstd_core.h"

typedef struct _process_os_interface
{
    void *_internalState;

    String* (*const console_get_args_utf8)(int argc, char** argv);
    void (*const console_set_utf8)(void);
    void (*const exit)(i32 code);
} _ProcOsInterface;
