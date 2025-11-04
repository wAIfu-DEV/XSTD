#pragma once

// Implementation of _FileOsInterface using Cstd

#include "xstd_file_os_int.h"

#include "stdio.h"
#include "errno.h"

void *__file_io_c_fstdout(void)
{
    return stdout;
}

void *__file_io_c_fstderr(void)
{
    return stderr;
}

void *__file_io_c_fstdin(void)
{
    return stdin;
}

int __file_io_c_fopen(void **stream, const char *fileName, const char *mode)
{
#ifdef _MSC_VER
    return (int)fopen_s((FILE **)stream, fileName, mode);
#else
    *(FILE **)stream = fopen(fileName, mode);
    return *(FILE **)stream == NULL ? 0 : errno;
#endif
}

int __file_io_c_fclose(void *stream)
{
    return fclose((FILE *)stream);
}

int __file_io_c_fgetc(void *stream)
{
    return fgetc((FILE *)stream);
}

int __file_io_c_fseek(void *stream, long off, int origin)
{
    return fseek((FILE *)stream, off, origin);
}

long __file_io_c_ftell(void *stream)
{
    return ftell((FILE *)stream);
}

int __file_io_c_fputc(int c, void *stream)
{
    return fputc(c, (FILE *)stream);
}

int __file_io_c_fflush(void *stream)
{
    return fflush((FILE *)stream);
}

int __file_io_c_feof(void *stream)
{
    return feof((FILE *)stream);
}

const _FileOsInterface __file_os_int = {
    ._internalState = NULL,

    .fstdout = __file_io_c_fstdout,
    .fstderr = __file_io_c_fstderr,
    .fstdin = __file_io_c_fstdin,
    .open = __file_io_c_fopen,
    .close = __file_io_c_fclose,
    .getc = __file_io_c_fgetc,
    .seek = __file_io_c_fseek,
    .tell = __file_io_c_ftell,
    .putc = __file_io_c_fputc,
    .flush = __file_io_c_fflush,
    .eof = __file_io_c_feof,
};
