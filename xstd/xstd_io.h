#pragma once

#include "xstd_file.h"

#define IoStdout ((File){._handle = stdout, ._valid = 1})
#define IoStderr ((File){._handle = stderr, ._valid = 1})
#define IoStdin ((File){._handle = stdin, ._valid = 1})

/**
 * @brief Prints text to stdout and displays it on the console.
 *
 * @param text
 */
void io_print(ConstStr text)
{
    File f = IoStdout;

    if (!text)
    {
        file_write_null(&f);
        return;
    }

    while (*text)
    {
        fputc(*text, f._handle);
        ++text;
    }
}

void io_print_int(const i64 i)
{
    File f = IoStdout;
    file_write_int(&f, i);
}

void io_print_uint(const u64 i)
{
    File f = IoStdout;
    file_write_uint(&f, i);
}

void io_print_float(const f64 flt, const u64 precision)
{
    File f = IoStdout;
    file_write_f64(&f, flt, precision);
}

void io_print_char(const char c)
{
    File f = IoStdout;
    file_write_char(&f, c);
}

/**
 * @brief Prints text to stdout with a newline and displays it on the console.
 *
 * @param text
 */
void io_println(ConstStr text)
{
    File f = IoStdout;

    if (!text)
    {
        file_write_null(&f);
        fputc('\n', f._handle);
        return;
    }

    while (*text)
    {
        fputc(*text, f._handle);
        ++text;
    }
    fputc('\n', f._handle);
}

/**
 * @brief Prints text to stderr and displays it on the console.
 *
 * @param text
 */
void io_printerr(ConstStr text)
{
    File f = IoStderr;

    if (!text)
    {
        file_write_null(&f);
        return;
    }

    while (*text)
    {
        fputc(*text, f._handle);
        ++text;
    }
    fflush(f._handle);
}

/**
 * @brief Prints text to stderr with a newline and displays it on the console.
 *
 * @param text
 */
void io_printerrln(ConstStr text)
{
    File f = IoStderr;

    if (!text)
    {
        file_write_null(&f);
        fputc('\n', f._handle);
        return;
    }

    while (*text)
    {
        fputc(*text, f._handle);
        ++text;
    }
    fputc('\n', f._handle);
    fflush(f._handle);
}

/**
 * @brief Crashes the program if `condition` is 0 (false) and prints `falseMessage` to stderr.
 *
 * ```
 * x_assert(0 == 0, "Somehow, 0 is not equal to 0.");
 * ```
 * @param text
 */
void x_assert(const ibool condition, ConstStr falseMessage)
{
    if (condition)
        return;
    io_printerr("[ASSERT FAILURE]: ");
    io_printerrln(falseMessage);

    while (true)
        exit(1);
}

/**
 * @brief Crashes the program if `err` is not ERR_OK and prints `isErrMessage` to stderr.
 *
 * ```
 * ResultFile res = file_open("example.txt", FileOpenModes.READ);
 * x_assertErr(res.error, "Failed to open the file.");
 * ```
 * @param text
 */
void x_assertErr(const Error err, ConstStr isErrMessage)
{
    if (err == ERR_OK)
        return;

    io_printerr("[ASSERT ERR FAILURE]");
    io_printerr("(ERROR: ");
    io_printerr(ErrorToString(err));
    io_printerr("): ");
    io_printerrln(isErrMessage);

    while (true)
        exit(1);
}

/**
 * @brief Crashes the program if `a` and `b` are different and prints `falseMessage` to stderr.
 *
 * ```
 * x_assertStrEq("0", "0", "Somehow, \"0\" is not equal to \"0\".");
 * ```
 * @param text
 */
void x_assertStrEq(ConstStr a, ConstStr b, ConstStr falseMessage)
{
    if (string_equals(a, b))
        return;

    io_printerr("[ASSERT STR EQ FAILURE]: ");
    io_printerrln(falseMessage);

    while (true)
        exit(1);
}
