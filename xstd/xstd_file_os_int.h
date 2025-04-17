#pragma once

#include "stdio.h"

typedef struct _file_os_interface
{
    void *_internalState;

    FILE *(*fstdout)(void);
    FILE *(*fstderr)(void);
    FILE *(*fstdin)(void);

#ifdef _MSC_VER
    errno_t (*open)(FILE **stream, const char *fileName, const char *mode);
#else
    FILE *(*open)(const char *fileName, const char *mode);
#endif

    int (*close)(FILE *stream);
    size_t (*read)(void *buff, size_t elemSize, size_t elemCount, FILE *stream);
    int (*seek)(FILE *stream, long off, int origin);
    long (*tell)(FILE *stream);
    int (*putc)(int c, FILE *stream);
    int (*flush)(FILE *stream);
    int (*eof)(FILE *stream);
} _FileOsInterface;

FILE *__file_io_c_fstdout(void)
{
    return stdout;
}

FILE *__file_io_c_fstderr(void)
{
    return stderr;
}

FILE *__file_io_c_fstdin(void)
{
    return stdin;
}

_FileOsInterface __file_os_int = {
    ._internalState = NULL,

    .fstdout = __file_io_c_fstdout,
    .fstderr = __file_io_c_fstderr,
    .fstdin = __file_io_c_fstdin,

#ifdef _MSC_VER
    .open = fopen_s,
#else
    .open = fopen,
#endif

    .close = fclose,
    .read = fread,
    .seek = fseek,
    .tell = ftell,
    .putc = fputc,
    .flush = fflush,
    .eof = feof,
};

void file_set_io_interface(_FileOsInterface interface)
{
    __file_os_int = interface;
}
