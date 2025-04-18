#pragma once

#include "stdio.h"

typedef struct _file_os_interface
{
    void *_internalState;

    void *(*fstdout)(void);
    void *(*fstderr)(void);
    void *(*fstdin)(void);
    int (*open)(void **stream, const char *fileName, const char *mode);
    int (*close)(void *stream);
    int (*getc)(void *stream);
    int (*seek)(void *stream, long off, int origin);
    long (*tell)(void *stream);
    int (*putc)(int c, void *stream);
    int (*flush)(void *stream);
    int (*eof)(void *stream);
} _FileOsInterface;
