#pragma once

// Implementation of _FileOsInterface using Cstd

#include "xstd_file_os_int.h"

#include "stdio.h"
#include "errno.h"

static void *_fosint_cstd_fstdout(void)
{
    return stdout;
}

static void *_fosint_cstd_fstderr(void)
{
    return stderr;
}

static void *_fosint_cstd_fstdin(void)
{
    return stdin;
}

static int _fosint_cstd_fopen(void **stream, const char *fileName, const char *mode)
{
    // Default fopen function differs by compiler type and/or OS
#ifdef _MSC_VER
    return (int)fopen_s((FILE **)stream, fileName, mode);
#else
    *(FILE **)stream = fopen(fileName, mode);
    return *(FILE **)stream != NULL ? 0 : errno;
#endif
}

static int _fosint_cstd_fclose(void *stream)
{
    return fclose((FILE *)stream);
}

static int _fosint_cstd_fgetc(void *stream)
{
    return fgetc((FILE *)stream);
}

static int _fosint_cstd_fseek(void *stream, long off, int origin)
{
    return fseek((FILE *)stream, off, origin);
}

static long _fosint_cstd_ftell(void *stream)
{
    return ftell((FILE *)stream);
}

static int _fosint_cstd_fputc(int c, void *stream)
{
    return fputc(c, (FILE *)stream);
}

static int _fosint_cstd_fflush(void *stream)
{
    return fflush((FILE *)stream);
}

static int _fosint_cstd_feof(void *stream)
{
    return feof((FILE *)stream);
}

static const _FileOsInterface _file_os_int_stdlib = {
    ._internalState = NULL,

    .fstdout = _fosint_cstd_fstdout,
    .fstderr = _fosint_cstd_fstderr,
    .fstdin = _fosint_cstd_fstdin,

    .open = _fosint_cstd_fopen,
    .close = _fosint_cstd_fclose,
    .getc = _fosint_cstd_fgetc,
    .seek = _fosint_cstd_fseek,
    .tell = _fosint_cstd_ftell,
    .putc = _fosint_cstd_fputc,
    .flush = _fosint_cstd_fflush,
    .eof = _fosint_cstd_feof,
};
