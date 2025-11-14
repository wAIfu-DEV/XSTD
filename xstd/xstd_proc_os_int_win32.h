#pragma once

#include "xstd_core.h"
#include "xstd_proc_os_int.h"
#include "xstd_win32.h"

static inline void _proc_os_win32_exit(i32 code) {
    ExitProcess(code);
}

static inline void _proc_os_win32_console_set_utf8(void)
{
    SetConsoleOutputCP(65001);
}

static inline String* _proc_os_win32_console_get_args_utf8(i32 argc, String* argv)
{
    (void)argv;

    Utf16Str* utf16Args = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!utf16Args)
    {
        return NULL;
    }

    // This feels icky, should we take in a allocator as arg instead?
    Allocator* a = default_allocator();
    HeapStr* utf8Args = a->alloc(a, sizeof(HeapStr) * (u64)argc);

    for (i64 i = 0; i < (i64)argc; ++i)
    {
        ResultOwnedStr res = utf16_to_utf8(a, utf16Args[i]);
        if (res.error.code)
            return NULL; // here we return null, if one or more args fail to
                         // get parsed, we cannot return invalid args back to
                         // the user.
        utf8Args[i] = res.value;
    }
    return utf8Args;
}

static const _ProcOsInterface _proc_os_int_win32 = {
    ._internalState = NULL,

    .console_get_args_utf8 = &_proc_os_win32_console_get_args_utf8,
    .console_set_utf8 = &_proc_os_win32_console_set_utf8,
    .exit = &_proc_os_win32_exit,
};
