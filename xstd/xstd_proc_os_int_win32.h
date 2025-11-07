#pragma once

#include "xstd_core.h"
#include "xstd_proc_os_int.h"
#include "xstd_win32.h"

static inline void _win32_exit(i32 code) {
    ExitProcess(code);
}

static const _ProcOsInterface _proc_os_int_win32 = {
    ._internalState = NULL,

    .exit = &_win32_exit,
};
