#pragma once

#include "xstd_core.h"

typedef struct _file_os_interface
{
    void *_internalState;

    void *(*const fstdout)(void);
    void *(*const fstderr)(void);
    void *(*const fstdin)(void);
    int (*const open)(void **stream, const char *fileName, const char *mode);
    int (*const close)(void *stream);
    int (*const getc)(void *stream);
    u64 (*const read)(void *stream, void *dst, u64 bytes);
    int (*const seek)(void *stream, long off, int origin);
    long (*const tell)(void *stream);
    int (*const putc)(int c, void *stream);
    u64 (*const write)(void *stream, const void *src, u64 bytes);
    int (*const flush)(void *stream);
    int (*const eof)(void *stream);
} _FileOsInterface;
