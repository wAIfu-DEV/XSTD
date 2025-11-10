#pragma once

#include "xstd_alloc.h"
#include "xstd_alloc_win32.h"
#include "xstd_alloc_default.h"
#include "xstd_core.h"
#include "xstd_file_os_int.h"
#include "xstd_win32.h"

#include "xstd_utf16.h"
#include "xstd_utf8.h"

typedef struct {
    _w32_handle h;
    _w32_bool eof; // TODO: replace this, this is dumb
} _Win32File;

static _Win32File _w32_stdout_file = {0};
static _Win32File _w32_stderr_file = {0};
static _Win32File _w32_stdin_file = {0};

static void* _fosint_win32_stdout(void) {
    if (!_w32_stdout_file.h)
        _w32_stdout_file.h = GetStdHandle(_WIN_32_STD_OUTPUT_HANDLE);
    return &_w32_stdout_file;
}

static void* _fosint_win32_stderr(void) {
    if (!_w32_stderr_file.h)
        _w32_stderr_file.h = GetStdHandle(_WIN_32_STD_ERROR_HANDLE);
    return &_w32_stderr_file;
}

static void* _fosint_win32_stdin(void)  {
    if (!_w32_stdin_file.h)
        _w32_stdin_file.h = GetStdHandle(_WIN_32_STD_INPUT_HANDLE);
    return &_w32_stdin_file;
}

static int _fosint_win32_open(void** stream, const char* fileName, const char* mode) {
    _w32_dword access = 0, creation = 0;
    ibool allowRead = false;

    if (mode) {
        for (const char *m = mode; *m; ++m) {
            if (*m == '+') {
                allowRead = true;
                break;
            }
        }
    }

    if (mode[0] == 'r') {
        access = _WIN_32_GENERIC_READ;
        if (allowRead)
            access |= _WIN_32_GENERIC_WRITE;
        creation = _WIN_32_OPEN_EXISTING;
    }
    else if (mode[0] == 'w') {
        access = _WIN_32_GENERIC_WRITE;
        if (allowRead)
            access |= _WIN_32_GENERIC_READ;
        creation = _WIN_32_CREATE_ALWAYS;
    }
    else if (mode[0] == 'a') {
        access = _WIN_32_GENERIC_WRITE;
        if (allowRead)
            access |= _WIN_32_GENERIC_READ;
        creation = _WIN_32_OPEN_EXISTING;
    }
    else {
        return -3;
    }

    Allocator* a = &_win32_allocator;
    ResultUtf16OwnedStr convRes = utf8_to_utf16(a, fileName);
    if (convRes.error.code)
        return -2;

    Utf16OwnedStr utf16FileName = convRes.value;

    _w32_handle h = CreateFileW(utf16FileName, access, 0, 0, creation, 0, 0);
    if (h == _WIN_32_INVALID_HANDLE_VALUE) {
        if (mode[0] == 'a') {
            h = CreateFileW(utf16FileName, access, 0, 0, _WIN_32_CREATE_ALWAYS, 0, 0);
        }

        if (h == _WIN_32_INVALID_HANDLE_VALUE) {
            return -1;
        }
    }

    _Win32File* f = (_Win32File*)HeapAlloc(GetProcessHeap(), 0, sizeof(_Win32File));
    f->h = h; f->eof = 0;

    if (mode[0] == 'a') {
        SetFilePointerEx(f->h, 0, 0, _WIN_32_FILE_END);
    }

    *stream = f;
    return 0;
}

static int _fosint_win32_close(void* stream) {
    _Win32File* f = (_Win32File*)stream;
    _w32_bool ok = CloseHandle(f->h);
    HeapFree(GetProcessHeap(), 0, f);
    return ok ? 0 : -1;
}

static int _fosint_win32_getc(void* stream) {
    _Win32File* f = (_Win32File*)stream;
    unsigned char c;
    _w32_dword read = 0;
    _w32_bool ok = ReadFile(f->h, &c, 1, &read, 0);
    if (!ok || read == 0) {
        f->eof = 1;
        return -1;
    }
    return c;
}

static int _fosint_win32_putc(int c, void* stream) {
    _Win32File* f = (_Win32File*)stream;
    f->eof = 0;
    unsigned char ch = (unsigned char)c;
    _w32_dword written;
    _w32_bool ok = WriteFile(f->h, &ch, 1, &written, 0);
    return ok && written == 1 ? c : -1;
}

static int _fosint_win32_seek(void* stream, long off, int origin) {
    _Win32File* f = (_Win32File*)stream;
    f->eof = 0;
    long long newpos;
    _w32_bool ok = SetFilePointerEx(f->h, off, &newpos, origin);
    return ok ? 0 : -1;
}

static long _fosint_win32_tell(void* stream) {
    _Win32File* f = (_Win32File*)stream;
    long long pos = 0;
    SetFilePointerEx(f->h, 0, &pos, _WIN_32_FILE_CURRENT);
    return (long)pos;
}

static int _fosint_win32_fflush(void* stream) {
    (void)stream;
    // Win32 WriteFile is unbuffered at this level, so nothing to flush
    return 0;
}

static int _fosint_win32_feof(void* stream) {
    return ((_Win32File*)stream)->eof;
}

static const _FileOsInterface _file_os_int_win32 = {
    ._internalState = NULL,

    .fstdout = _fosint_win32_stdout,
    .fstderr = _fosint_win32_stderr,
    .fstdin  = _fosint_win32_stdin,

    .open    = _fosint_win32_open,
    .close   = _fosint_win32_close,
    .getc    = _fosint_win32_getc,
    .seek    = _fosint_win32_seek,
    .tell    = _fosint_win32_tell,
    .putc    = _fosint_win32_putc,
    .flush   = _fosint_win32_fflush,
    .eof     = _fosint_win32_feof,
};
