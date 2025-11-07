#pragma once

#include "xstd_core.h"
#include "xstd_proc_os_int.h"

#include "stdlib.h"

static void _proc_os_stdlib_exit(i32 code) {
    exit(code);
}

static const _ProcOsInterface _proc_os_int_stdlib = {
    ._internalState = NULL,

    .exit = &_proc_os_stdlib_exit,
};
