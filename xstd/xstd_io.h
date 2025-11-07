#pragma once

#include "xstd_file.h"

#include "xstd_proc_os_int.h"
#include "xstd_proc_os_int_default.h"
#include "xstd_file_os_int_default.h"

#define IoStdout ((File){._handle = _default_file_os_int()->fstdout(), ._valid = true})
#define IoStderr ((File){._handle = _default_file_os_int()->fstderr(), ._valid = true})
#define IoStdin ((File){._handle = _default_file_os_int()->fstdin(), ._valid = true})

static inline void io_print_char(const char c)
{
    File f = IoStdout;
    file_write_char(&f, c);
}

/**
 * @brief Prints text to stdout and displays it on the console.
 *
 * @param text
 */
static inline void io_print(ConstStr text)
{
    File f = IoStdout;

    if (!text)
    {
        file_write_null(&f);
        return;
    }

    while (*text)
    {
        file_write_char(&f, *text);
        ++text;
    }
}

static inline void io_print_int(const i64 i)
{
    File f = IoStdout;
    file_write_int(&f, i);
}

static inline void io_print_uint(const u64 i)
{
    File f = IoStdout;
    file_write_uint(&f, i);
}

static inline void io_print_float(const f64 flt, const u64 precision)
{
    File f = IoStdout;
    file_write_f64(&f, flt, precision);
}

/**
 * @brief Prints text to stdout with a newline and displays it on the console.
 *
 * @param text
 */
static inline void io_println(ConstStr text)
{
    File f = IoStdout;

    if (!text)
    {
        file_write_null(&f);
        file_write_char(&f, '\n');
        return;
    }

    while (*text)
    {
        file_write_char(&f, *text);
        ++text;
    }
    file_write_char(&f, '\n');
}

/**
 * @brief Prints text to stderr and displays it on the console.
 *
 * @param text
 */
static inline void io_printerr(ConstStr text)
{
    File f = IoStderr;

    file_write_str(&f, "\x1b[1;31m");

    if (!text)
    {
        file_write_null(&f);
        return;
    }

    while (*text)
    {
        file_write_char(&f, *text);
        ++text;
    }

    file_write_str(&f, "\x1b[0m");
    file_flush(&f);
}

/**
 * @brief Prints text to stderr with a newline and displays it on the console.
 *
 * @param text
 */
static inline void io_printerrln(ConstStr text)
{
    File f = IoStderr;

    file_write_str(&f, "\x1b[1;31m");

    if (!text)
    {
        file_write_null(&f);
        file_write_char(&f, '\n');
        return;
    }

    while (*text)
    {
        file_write_char(&f, *text);
        ++text;
    }

    file_write_char(&f, '\n');
    file_write_str(&f, "\x1b[0m");
    file_flush(&f);
}

/**
 * @brief Reads a single line from stdin, returns a OwnedStr owned by the caller.
 * The string will be null-terminated, but does NOT include the trailing `\n` unless EOF.
 * Caller is responsible for freeing the string.
 *
 * @param alloc Allocator to use for allocating the buffer
 * @return OwnedStr or NULL on allocation failure or EOF
 */
static inline ResultOwnedStr io_read_line(Allocator *a)
{
    // TODO: Implement as generic file_read_line function

    if (!a)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("io", "io_read_line",
                     ERR_INVALID_PARAMETER, "null allocator"),
        };

    const _FileOsInterface* foi = _default_file_os_int();
    File f = IoStdin;

    u64 buffSize = 32;
    i8 *buffer = (i8*)a->alloc(a, buffSize);

    if (!buffer)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("io", "io_read_line",
                     ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    u64 len = 0;

    while (true)
    {
        int ch = foi->getc(f._handle);
        if (ch == -1 /* EOF */ || ch == '\n' || ch == '\r')
            break;

        if (len + 1 >= buffSize)
        {
            u64 newSize = buffSize + 32;
            i8 *newBuff = (i8*)a->realloc(a, buffer, newSize);
            if (!newBuff)
            {
                a->free(a, buffer);
                return (ResultOwnedStr){
                    .value = NULL,
                    .error = X_ERR_EXT("io", "io_read_line",
                             ERR_OUT_OF_MEMORY, "alloc failure"),
                };
            }
            buffer = newBuff;
            buffSize = newSize;
        }
        buffer[len++] = (i8)ch;
    }

    if (len == 0 && foi->eof(f._handle))
    {
        a->free(a, buffer);
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("io", "io_read_line",
                     ERR_FILE_CANT_READ, "cannot read from file"),
        };
    }
    buffer[len] = 0;

    return (ResultOwnedStr){
        .value = buffer,
        .error = X_ERR_OK,
    };
}

static inline void crash(i16 code)
{
    const _ProcOsInterface* poi = _default_proc_os_int();
    while (true)
        poi->exit(code);
}

static inline void crash_print(ConstStr errMsg, i16 code)
{
    File f = IoStderr;

    // We use file_write here to not spam file_flush
    file_write_str(&f, "\x1b[1;31m");
    file_write_str(&f, "[CRASH]: ");
    io_printerrln(errMsg);

    crash(code);
}

static inline void crash_print_error(Error err, ConstStr errMsg, i16 code)
{
    File f = IoStderr;

    // We use file_write here to not spam file_flush
    file_write_str(&f, "\x1b[1;31m");
    file_write_str(&f, "[CRASH]\n- code: ");
    file_write_str(&f, ErrorToString(err.code));
    file_write_str(&f, "\n- desc: ");
    io_printerrln(errMsg);

    crash(code);
}

/**
 * @brief Crashes the program if `condition` is 0 (false) and prints `falseMessage` to stderr.
 *
 * ```
 * assert_true(0 == 0, "Somehow, 0 is not equal to 0.");
 * ```
 * @param text
 */
static inline void assert_true(const ibool condition, ConstStr falseMessage)
{
    if (condition)
        return;

    File f = IoStderr;

    file_write_str(&f, "\x1b[1;31m");
    file_write_str(&f, "[ASSERT FAILURE]: ");
    io_printerrln(falseMessage);

    crash(1);
}

/**
 * @brief Crashes the program if `err` is not ERR_OK and prints `isErrMessage` to stderr.
 *
 * ```
 * ResultFile res = file_open("example.txt", FileOpenModes.READ);
 * assert_ok(res.error, "Failed to open the file.");
 * ```
 * @param text
 */
static inline void assert_ok(const Error err, ConstStr isErrMessage)
{
    if (err.code == ERR_OK)
        return;

    File f = IoStderr;

    // We use file_write here to not spam file_flush
    file_write_str(&f, "\x1b[1;31m");
    file_write_str(&f, "[ASSERT ERR FAILURE]\n- code: ");
    file_write_str(&f, ErrorToString(err.code));
    file_write_str(&f, "\n- msg: ");
    file_write_str(&f, err.msg);
    file_write_str(&f, "\n- desc: ");
    io_printerrln(isErrMessage);

    crash(1);
}

/**
 * @brief Crashes the program if `a` and `b` are different and prints `falseMessage` to stderr.
 *
 * ```
 * assert_str_eq("0", "0", "Somehow, \"0\" is not equal to \"0\".");
 * ```
 * @param text
 */
static inline void assert_str_eq(ConstStr a, ConstStr b, ConstStr falseMessage)
{
    if (string_equals(a, b))
        return;

    File f = IoStderr;

    file_write_str(&f, "\x1b[1;31m");
    file_write_str(&f, "[ASSERT STR EQ FAILURE]: ");
    io_printerrln(falseMessage);

    crash(1);
}
