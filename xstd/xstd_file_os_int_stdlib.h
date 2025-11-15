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

static u64 _fosint_cstd_fread(void *stream, void *dst, u64 bytes)
{
    if (!stream || !dst || bytes == 0)
        return 0;

    FILE *f = (FILE *)stream;
    unsigned char *out = (unsigned char *)dst;
    u64 totalRead = 0;

    while (totalRead < bytes)
    {
        size_t remaining = (size_t)((bytes - totalRead) > (u64)(size_t)-1 ? (size_t)-1 : (bytes - totalRead));
        if (remaining == 0)
            break;

        size_t read = fread(out + totalRead, 1, remaining, f);
        totalRead += read;

        if (read != remaining)
            break;
    }

    return totalRead;
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

static u64 _fosint_cstd_fwrite(void *stream, const void *src, u64 bytes)
{
    if (!stream || !src || bytes == 0)
        return 0;

    FILE *f = (FILE *)stream;
    const unsigned char *data = (const unsigned char *)src;
    u64 totalWritten = 0;

    while (totalWritten < bytes)
    {
        size_t remaining = (size_t)((bytes - totalWritten) > (u64)(size_t)-1 ? (size_t)-1 : (bytes - totalWritten));
        if (remaining == 0)
            break;

        size_t written = fwrite(data + totalWritten, 1, remaining, f);
        totalWritten += written;

        if (written != remaining)
            break;
    }

    return totalWritten;
}

static int _fosint_cstd_fflush(void *stream)
{
    return fflush((FILE *)stream);
}

static int _fosint_cstd_feof(void *stream)
{
    return feof((FILE *)stream);
}

static u64 _fosint_cstd_fsize(void *stream)
{
    // TODO: this will break with large files, prefer posix version

    const int seekEnd = 2;
    const int seekSet = 0;

    long ogTell = ftell((FILE *)stream);

    fseek((FILE*)stream, 0, seekEnd);
    long fileSize = ftell((FILE*)stream);

    fseek((FILE*)stream, ogTell, seekSet);
    return (u64)fileSize;
}

static const _FileOsInterface _file_os_int_stdlib = {
    ._internalState = NULL,

    .fstdout = _fosint_cstd_fstdout,
    .fstderr = _fosint_cstd_fstderr,
    .fstdin = _fosint_cstd_fstdin,

    .open = _fosint_cstd_fopen,
    .close = _fosint_cstd_fclose,
    .getc = _fosint_cstd_fgetc,
    .read = _fosint_cstd_fread,
    .seek = _fosint_cstd_fseek,
    .tell = _fosint_cstd_ftell,
    .putc = _fosint_cstd_fputc,
    .write = _fosint_cstd_fwrite,
    .flush = _fosint_cstd_fflush,
    .eof = _fosint_cstd_feof,
    .size = _fosint_cstd_fsize,
};
