#pragma once

#include "xstd_file.h"

#include "xstd_proc_os_int.h"
#include "xstd_proc_os_int_default.h"
#include "xstd_file_os_int_default.h"

static inline File* io_stdout(void)
{
    static File f = {0};
    if (!f._valid) {
        f._handle = _default_file_os_int()->fstdout();
        f._valid = true;
    }
    return &f;
}

static inline File* io_stderr(void)
{
    static File f = {0};
    if (!f._valid) {
        f._handle = _default_file_os_int()->fstderr();
        f._valid = true;
    }
    return &f;
}

static inline File* io_stdin(void)
{
    static File f = {0};
    if (!f._valid) {
        f._handle = _default_file_os_int()->fstderr();
        f._valid = true;
    }
    return &f;
}

static inline void io_print_char(const char c)
{
    file_write_char(io_stdout(), c);
}

/**
 * @brief Prints text to stdout and displays it on the console.
 *
 * @param text
 */
static inline void io_print(ConstStr text)
{
    File* f = io_stdout();

    if (!text)
    {
        file_write_null(f);
        return;
    }

    file_write_str(f, text);
}

static inline void io_print_int(const i64 i)
{
    file_write_int(io_stdout(), i);
}

static inline void io_print_uint(const u64 i)
{
    file_write_uint(io_stdout(), i);
}

static inline void io_print_float(const f64 flt, const u64 precision)
{
    file_write_f64(io_stdout(), flt, precision);
}

/**
 * @brief Prints text to stdout with a newline and displays it on the console.
 *
 * @param text
 */
static inline void io_println(ConstStr text)
{
    File* f = io_stdout();

    if (!text)
    {
        file_write_null(f);
        file_write_char(f, '\n');
        return;
    }

    file_write_str(f, text);
    file_write_char(f, '\n');
}

/**
 * @brief Prints text to stderr and displays it on the console.
 *
 * @param text
 */
static inline void io_printerr(ConstStr text)
{
    File *f = io_stderr();

    file_write_str(f, "\x1b[1;31m");

    if (!text)
    {
        file_write_null(f);
        return;
    }

    file_write_str(f, text);
    file_write_str(f, "\x1b[0m");
    file_flush(f);
}

/**
 * @brief Prints text to stderr with a newline and displays it on the console.
 *
 * @param text
 */
static inline void io_printerrln(ConstStr text)
{
    File *f = io_stderr();

    file_write_str(f, "\x1b[1;31m");

    if (!text)
    {
        file_write_null(f);
        file_write_str(f, "\n\x1b[0m");
        return;
    }

    file_write_str(f, text);
    file_write_str(f, "\n\x1b[0m");
    file_flush(f);
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
    File *f = io_stdin();

    u64 buffSize = 32;
    i8 *buffer = (i8*)a->alloc(a, buffSize);

    if (!buffer)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("io", "io_read_line",
                     ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    u64 len = 0;

    while(true)
    {
        int ch = foi->getc(f->_handle);
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

    if (len == 0 && foi->eof(f->_handle))
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
        .value = (char*)buffer,
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
    File *f = io_stderr();

    // We use file_write here to not spam file_flush
    file_write_str(f, "\x1b[1;31m");
    file_write_str(f, "[CRASH]: ");
    file_write_str(f, errMsg);
    file_write_str(f, "\n\x1b[0m");

    crash(code);
}

static inline void crash_print_error(Error err, ConstStr errMsg, i16 code)
{
    File *f = io_stderr();

    // We use file_write here to not spam file_flush
    file_write_str(f, "\x1b[1;31m");
    file_write_str(f, "[CRASH]\n- code: ");
    file_write_str(f, ErrorToString(err.code));
    file_write_str(f, "\n- desc: ");
    file_write_str(f, errMsg);
    file_write_str(f, "\n\x1b[0m");

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
static inline void assert_true(const Bool condition, ConstStr falseMessage)
{
    if (condition)
        return;

    File *f = io_stderr();

    file_write_str(f, "\x1b[1;31m");
    file_write_str(f, "[ASSERT FAILURE]: ");
    file_write_str(f, falseMessage);
    file_write_str(f, "\n\x1b[0m");

    crash(1);
}

/**
 * @brief Crashes the program if `err` is not ERR_OK and prints `isErrMessage` to stderr.
 *
 * ```
 * ResultFile res = file_open("example.txt", EnumFileOpenMode.READ);
 * assert_ok(res.error, "Failed to open the file.");
 * ```
 * @param text
 */
static inline void assert_ok(const Error err, ConstStr isErrMessage)
{
    if (err.code == ERR_OK)
        return;

    File *f = io_stderr();

    // We use file_write here to not spam file_flush
    file_write_str(f, "\x1b[1;31m");
    file_write_str(f, "[ASSERT ERR FAILURE]\n- code: ");
    file_write_str(f, ErrorToString(err.code));
    file_write_str(f, "\n- msg: ");
    file_write_str(f, err.msg);
    file_write_str(f, "\n- desc: ");
    file_write_str(f, isErrMessage);
    file_write_str(f, "\n\x1b[0m");

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

    File *f = io_stderr();

    file_write_str(f, "\x1b[1;31m");
    file_write_str(f, "[ASSERT STR EQ FAILURE]: ");
    file_write_str(f, falseMessage);
    file_write_str(f, "\n\x1b[0m");

    crash(1);
}

__attribute__((constructor))
static inline void _io_setmode_utf8(void)
{
    // Depending on OS may need to explicitly set console to utf8 mode
    _default_proc_os_int()->console_set_utf8();
}

static inline String* io_args_utf8(i32 argc, String* argv)
{
    return _default_proc_os_int()->console_get_args_utf8(argc, argv);
}
