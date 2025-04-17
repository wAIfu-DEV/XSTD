#pragma once

#include "xstd_file_os_int.h"

#include "stdio.h"

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

#ifdef _MSC_VER
errno_t __file_io_c_fopen_s(void **stream, const char *fileName, const char *mode)
{
    return fopen_s((FILE **)stream, fileName, mode);
}
#else
void *__file_io_c_fopen(const char *fileName, const char *mode)
{
    return (void *)fopen(fileName, mode);
}
#endif

int __file_io_c_fclose(void *stream)
{
    return fclose((FILE *)stream);
}

size_t __file_io_c_fread(void *buff, size_t elemSize, size_t elemCount, void *stream)
{
    return fread(buff, elemSize, elemCount, (FILE *)stream);
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

_FileOsInterface __file_os_int = {
    ._internalState = NULL,

    .fstdout = __file_io_c_fstdout,
    .fstderr = __file_io_c_fstderr,
    .fstdin = __file_io_c_fstdin,
#ifdef _MSC_VER
    .open = __file_io_c_fopen_s,
#else
    .open = __file_io_c_fopen,
#endif
    .close = __file_io_c_fclose,
    .read = __file_io_c_fread,
    .seek = __file_io_c_fseek,
    .tell = __file_io_c_ftell,
    .putc = __file_io_c_fputc,
    .flush = __file_io_c_fflush,
    .eof = __file_io_c_feof,
};

void file_set_io_interface(_FileOsInterface interface)
{
    __file_os_int = interface;
}
