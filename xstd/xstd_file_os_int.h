#pragma once

typedef struct _file_os_interface
{
    void *_internalState;

    void *(*const fstdout)(void);
    void *(*const fstderr)(void);
    void *(*const fstdin)(void);
    int (*const open)(void **stream, const char *fileName, const char *mode);
    int (*const close)(void *stream);
    int (*const getc)(void *stream);
    int (*const seek)(void *stream, long off, int origin);
    long (*const tell)(void *stream);
    int (*const putc)(int c, void *stream);
    int (*const flush)(void *stream);
    int (*const eof)(void *stream);
} _FileOsInterface;
