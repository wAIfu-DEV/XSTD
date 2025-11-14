#pragma once

#include "xstd_core.h"
#include "xstd_proc_os_int.h"

#include "stdlib.h"

static void _proc_os_stdlib_exit(i32 code) {
    exit(code);
}

static inline void _proc_os_stdlib_console_set_utf8(void)
{
    return;
}

static inline String* _proc_os_stdlib_console_get_args_utf8(i32 argc, String* argv)
{
    (void)argc;
    return argv;
}

static const _ProcOsInterface _proc_os_int_stdlib = {
    ._internalState = NULL,

    .console_get_args_utf8 = &_proc_os_stdlib_console_get_args_utf8,
    .console_set_utf8 = &_proc_os_stdlib_console_set_utf8,
    .exit = &_proc_os_stdlib_exit,
};
