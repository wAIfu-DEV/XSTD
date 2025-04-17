#pragma once

#include "stdio.h"

typedef struct _file_os_interface
{
    void *_internalState;

    void *(*fstdout)(void);
    void *(*fstderr)(void);
    void *(*fstdin)(void);

#ifdef _MSC_VER
    errno_t (*open)(void **stream, const char *fileName, const char *mode);
#else
    void *(*open)(const char *fileName, const char *mode);
#endif

    int (*close)(void *stream);
    size_t (*read)(void *buff, size_t elemSize, size_t elemCount, void *stream);
    int (*seek)(void *stream, long off, int origin);
    long (*tell)(void *stream);
    int (*putc)(int c, void *stream);
    int (*flush)(void *stream);
    int (*eof)(void *stream);
} _FileOsInterface;
