#pragma once

#include "xstd_file.h"

#define IoStdout ((File){._handle = __file_os_int.fstdout(), ._valid = 1})
#define IoStderr ((File){._handle = __file_os_int.fstderr(), ._valid = 1})
#define IoStdin ((File){._handle = __file_os_int.fstdin(), ._valid = 1})

void io_print_char(const char c)
{
    File f = IoStdout;
    file_write_char(&f, c);
}

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
        file_write_char(&f, *text);
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
        file_write_char(&f, *text);
        ++text;
    }
    file_flush(&f);
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
        file_write_char(&f, '\n');
        return;
    }

    while (*text)
    {
        file_write_char(&f, *text);
        ++text;
    }
    file_write_char(&f, '\n');
    file_flush(&f);
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

    File f = IoStderr;

    file_write_str(&f, "[ASSERT FAILURE]: ");
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

    File f = IoStderr;

    // We use file_write here to not spam file_flush
    file_write_str(&f, "[ASSERT ERR FAILURE](ERROR: ");
    file_write_str(&f, ErrorToString(err));
    file_write_str(&f, "): ");
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

    File f = IoStderr;

    file_write_str(&f, "[ASSERT STR EQ FAILURE]: ");
    io_printerrln(falseMessage);

    while (true)
        exit(1);
}
