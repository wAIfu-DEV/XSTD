#pragma once

#include "xstd_alloc_default.h"
#include "xstd_core.h"
#include "xstd_errcode.h"
#include "xstd_error.h"
#include "xstd_result.h"
#include "xstd_list.h"
#include "xstd_string.h"
#include "xstd_buffer.h"
#include "xstd_file_os_int.h"
#include "xstd_file_os_int_default.h"

/**
 * @typedef {u8} FileOpenMode
 * Symbolic value that selects how a file should be opened.
 * Use the predefined members of `FileOpenModes` when calling `file_open`.
 */
typedef u8 FileOpenMode;

#define FILE_OPENMODE_READ 0
#define FILE_OPENMODE_WRITE 1
#define FILE_OPENMODE_READWRITE 2
#define FILE_OPENMODE_TRUNC_READWRITE 3
#define FILE_OPENMODE_APPEND 4

/**
 * Named constants describing the supported `FileOpenMode` values for `file_open`.
 * @type {{READ: FileOpenMode, WRITE: FileOpenMode, READWRITE: FileOpenMode, APPEND: FileOpenMode}}
 */
static const struct _file_open_mode
{
    const FileOpenMode READ;
    const FileOpenMode WRITE;
    const FileOpenMode READWRITE;
    const FileOpenMode TRUNC_READWRITE;
    const FileOpenMode APPEND;
} FileOpenModes = {
    .READ = FILE_OPENMODE_READ,
    .WRITE = FILE_OPENMODE_WRITE,
    .READWRITE = FILE_OPENMODE_READWRITE,
    .TRUNC_READWRITE = FILE_OPENMODE_TRUNC_READWRITE,
    .APPEND = FILE_OPENMODE_APPEND,
};

/**
 * @typedef File
 * Lightweight wrapper that tracks a platform specific file handle and its validity.
 * @property {void*} _handle - Native file descriptor managed by the OS abstraction layer.
 * @property {ibool} _valid - Non-zero when `_handle` refers to an open file.
 */
typedef struct _file
{
    void *_handle;
    ibool _valid;
} File;

/**
 * @typedef ResultFile
 * Aggregates the outcome of an operation that attempts to produce a `File`.
 * @property {File} value - Populated when `error.code == ERR_OK`, otherwise zero-initialized.
 * @property {Error} error - Rich error descriptor explaining why the call failed.
 */
typedef struct _result_file
{
    File value;
    Error error;
} ResultFile;

static inline const char *_file_openmode_to_str(const FileOpenMode mode)
{
    switch (mode)
    {
    case FILE_OPENMODE_READ:
        return "rb";
    case FILE_OPENMODE_WRITE:
        return "wb";
    case FILE_OPENMODE_READWRITE:
        return "r+b";
    case FILE_OPENMODE_TRUNC_READWRITE:
        return "w+b";
    case FILE_OPENMODE_APPEND:
        return "a+b";
    default:
        return NULL;
    }
}

/**
 * Opens a file using the default operating system backend.
 * @example
 * ```c
 * ResultFile res = file_open("test.txt", FileOpenModes.READ);
 * if (res.error.code == ERR_OK) {
 *     File file = res.value;
 *     file_close(&file);
 * }
 * ```
 * @param path Null-terminated path to the file to open.
 * @param mode One of the values exposed on `FileOpenModes`.
 * @returns Result object containing the opened file or an error description.
 */
static inline ResultFile file_open(ConstStr path, const FileOpenMode mode)
{
    if (!path)
        return (ResultFile){
            .value = {0},
            .error = X_ERR_EXT("file", "file_open", ERR_INVALID_PARAMETER, "null path"),
        };

    const char *openArg = _file_openmode_to_str(mode);

    if (openArg == NULL)
        return (ResultFile){
            .value = {0},
            .error = X_ERR_EXT("file", "file_open", ERR_WOULD_NULL_DEREF, "open mode match failure"),
        };

    void *f;

    int err = _default_file_os_int()->open(&f, path, openArg);

    if (err)
    {
        Error newErr = X_ERR_EXT("file", "file_open",
            ERR_FILE_CANT_OPEN, "open failure");

        switch (err)
        {
        // ENOENT
        case 2:
            newErr = X_ERR_EXT("file", "file_open",
                ERR_FILE_NOT_FOUND, "file not found");
            break;
        default:
            break;
        }

        return (ResultFile){
            .value = {0},
            .error = newErr,
        };
    }

    return (ResultFile){
        .value = (File){
            ._handle = f,
            ._valid = 1,
        },
        .error = X_ERR_OK,
    };
}

/**
 * Closes a file handle obtained from `file_open`.
 * @example
 * ```c
 * ResultFile res = file_open("example.txt", FileOpenModes.READ);
 * if (res.error.code == ERR_OK) {
 *     File file = res.value;
 *     file_close(&file);
 * }
 * ```
 * @param file Handle to close; ignored when `NULL` or already invalid.
 */
static inline void file_close(File *file)
{
    if (!file || !file->_valid)
        return;

    file->_valid = false;
    _default_file_os_int()->close(file->_handle);
}

/**
 * Determines the number of bytes currently stored in a file.
 * @example
 * ```c
 * File file = file_open("example.txt", FileOpenModes.READ).value;
 * u64 size = file_size(&file);
 * ```
 * @param file Open file handle whose size should be queried.
 * @returns File length in bytes, or `0` when the handle is invalid.
 */
static inline u64 file_size(File *file)
{
    if (!file || !file->_valid)
        return 0;

    const int seekEnd = 2;
    const int seekSet = 0;

    const _FileOsInterface* foi = _default_file_os_int();

    long ogTell = foi->tell(file->_handle);

    foi->seek(file->_handle, 0, seekEnd);
    long fileSize = foi->tell(file->_handle);

    foi->seek(file->_handle, ogTell, seekSet);
    return (u64)fileSize;
}

static inline u64 _file_read_internal(File *f, i8 *buff, const u64 nBytes, ibool terminate)
{
    if (!f || !nBytes || !buff)
        return 0;

    const _FileOsInterface* foi = _default_file_os_int();
    u64 readPos = 0;

    while (readPos < nBytes)
    {
        int read = foi->getc(f->_handle);
        if (read < 0)
            break;

        buff[readPos] = (i8)read;
        ++readPos;
    }

    if (terminate)
        buff[readPos] = 0;

    return readPos;
}

/**
 * Adjusts the file cursor relative to the specified origin.
 * @param file Open file handle.
 * @param offset Byte offset relative to `origin`.
 * @param origin One of the standard seek constants (0 = start, 1 = current, 2 = end).
 * @returns `ERR_OK` on success or an error detailing the failure.
 */
static inline Error file_seek(File *file, const i64 offset, const i32 origin)
{
    if (!file || !file->_valid)
        return X_ERR_EXT("file", "file_seek",
            ERR_INVALID_PARAMETER, "null or invalid file");

    const int rc = _default_file_os_int()->seek(file->_handle, (long)offset, (int)origin);
    if (rc != 0)
        return X_ERR_EXT("file", "file_seek",
            ERR_FAILED, "seek failure");

    return X_ERR_OK;
}

/**
 * Reports the current cursor position of a file handle.
 * @param file Open file handle.
 * @returns Result containing the absolute byte offset or an error descriptor.
 */
static inline ResultU64 file_tell(File *file)
{
    if (!file || !file->_valid)
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("file", "file_tell",
                ERR_INVALID_PARAMETER, "null or invalid file"),
        };

    const long pos = _default_file_os_int()->tell(file->_handle);
    if (pos < 0)
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("file", "file_tell",
                ERR_FAILED, "tell failure"),
        };

    return (ResultU64){
        .value = (u64)pos,
        .error = X_ERR_OK,
    };
}

/**
 * Repositions the file cursor to the beginning of the file.
 * @param file Open file handle.
 * @returns `ERR_OK` on success or the error returned by `file_seek`.
 */
static inline Error file_rewind(File *file)
{
    const int seekSet = 0;
    return file_seek(file, 0, seekSet);
}


/**
 * Reads up to `nBytes` from the file into a newly allocated `HeapBuff`.
 * The caller owns the returned buffer and must release it with the same allocator.
 * @example
 * ```c
 * while (!file_is_eof(file)) {
 *     ResultOwnedBuff res = file_read_bytes(&allocator, file, 256);
 *     if (res.error.code != ERR_OK) {
 *         break;
 *     }
 *     HeapBuff chunk = res.value;
 *     // Process chunk.bytes / chunk.size
 *     buffer_free(&allocator, &chunk);
 * }
 * ```
 * @param a Allocator responsible for creating and freeing the buffer.
 * @param file Open file handle to read from.
 * @param nBytes Maximum number of bytes to read before stopping.
 * @returns Result that wraps the buffer when successful.
 */
static inline ResultOwnedBuff file_read_bytes(Allocator *a, File *file, const u64 nBytes)
{
    if (!a || !file || !file->_valid)
        return (ResultOwnedBuff){
            .value = (HeapBuff){.bytes = NULL, .size = 0},
            .error = X_ERR_EXT("file", "file_read_bytes",
                ERR_INVALID_PARAMETER, "null arg"),
        };

    i8 *newBuff = (i8*)a->alloc(a, nBytes);

    if (!newBuff)
        return (ResultOwnedBuff){
            .value = (HeapBuff){.bytes = NULL, .size = 0},
            .error = X_ERR_EXT("file", "file_read_bytes",
                ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    u64 readSize = _file_read_internal(file, newBuff, nBytes, false);

    if (readSize == 0)
    {
        a->free(a, newBuff);
        return (ResultOwnedBuff){
            .value = (HeapBuff){.bytes = NULL, .size = 0},
            .error = X_ERR_EXT("file", "file_read_bytes",
                ERR_FILE_CANT_READ, "read size mismatch"),
        };
    }

    return (ResultOwnedBuff){
        .value = (HeapBuff){.bytes = newBuff, .size = readSize},
        .error = X_ERR_OK,
    };
}

/**
 * Reads up to `nBytes` from the file and returns a null-terminated string.
 * The caller is responsible for freeing the allocated string with the allocator.
 * @example
 * ```c
 * while (!file_is_eof(file)) {
 *     ResultOwnedStr res = file_read_str(&allocator, file, 256);
 *     if (res.error.code != ERR_OK) {
 *         break;
 *     }
 *     OwnedStr chunk = res.value;
 *     // Process chunk
 *     allocator.free(&allocator, chunk);
 * }
 * ```
 * @param a Allocator used to allocate and later release the string.
 * @param file Open file handle to read from.
 * @param nBytes Maximum number of bytes to read before stopping.
 * @returns Result wrapping the allocated string on success.
 */
static inline ResultOwnedStr file_read_str(Allocator *a, File *file, const u64 nBytes)
{
    if (!a || !file || !file->_valid)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("file", "file_read_str",
                ERR_INVALID_PARAMETER, "null arg"),
        };

    HeapStr newStr = (HeapStr)a->alloc(a, nBytes + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("file", "file_read_str",
                ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    u64 readSize = _file_read_internal(file, newStr, nBytes, true);

    if (readSize == 0)
    {
        a->free(a, newStr);
        return (ResultOwnedStr){
            .error = X_ERR_EXT("file", "file_read_str",
                ERR_FILE_CANT_READ, "read size mismatch"),
        };
    }

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * Convenience variant of `file_read_str` that returns `NULL` on failure instead of a result object.
 * The returned string is null-terminated and must be released with the allocator.
 * @example
 * ```c
 * OwnedStr chunk = file_read_str_unsafe(&allocator, file, 256);
 * if (chunk != NULL) {
 *     // Process chunk
 *     allocator.free(&allocator, chunk);
 * }
 * ```
 * @param a Allocator used to obtain the string storage.
 * @param file Open file handle to read from.
 * @param nBytes Maximum number of bytes to read before termination.
 * @returns Newly allocated string, or `NULL` when reading fails.
 */
static inline OwnedStr file_read_str_unsafe(Allocator *a, File *file, u64 nBytes)
{
    if (nBytes == 0)
        return NULL;

    HeapStr newStr = (HeapStr)a->alloc(a, nBytes + 1);

    if (!newStr)
        return NULL;

    u64 readSize = _file_read_internal(file, newStr, nBytes, true);

    if (readSize == 0)
    {
        a->free(a, newStr);
        return NULL;
    }
    return newStr;
}

/**
 * Convenience variant of `file_read_bytes` that returns an empty buffer on failure.
 * Ownership of the returned buffer stays with the caller, who must free it with the allocator.
 * @example
 * ```c
 * HeapBuff chunk = file_read_bytes_unsafe(&allocator, file, 256);
 * if (chunk.bytes != NULL) {
 *     // Process chunk.bytes / chunk.size
 *     buffer_free(&allocator, &chunk);
 * }
 * ```
 * @param a Allocator responsible for managing the buffer memory.
 * @param file Open file handle to read from.
 * @param nBytes Maximum number of bytes to read.
 * @returns Buffer that may contain fewer than `nBytes` bytes when EOF is reached.
 */
static inline HeapBuff file_read_bytes_unsafe(Allocator *a, File *file, u64 nBytes)
{
    if (nBytes == 0)
        return (HeapBuff){.bytes = NULL, .size = 0};

    i8 *newBuff = (i8*)a->alloc(a, nBytes);

    if (!newBuff)
        return (HeapBuff){.bytes = NULL, .size = 0};

    u64 readSize = _file_read_internal(file, newBuff, nBytes, false);

    if (readSize == 0)
    {
        a->free(a, newBuff);
        return (HeapBuff){.bytes = NULL, .size = 0};
    }

    return (HeapBuff){.bytes = newBuff, .size = readSize};
}

/**
 * Reads the entire file into a null-terminated string allocated with the provided allocator.
 * The caller must release the string once it is no longer needed.
 * @example
 * ```c
 * ResultOwnedStr res = file_readall_str(&allocator, file);
 * if (res.error.code == ERR_OK) {
 *     OwnedStr contents = res.value;
 *     // Use contents
 *     allocator.free(&allocator, contents);
 * }
 * ```
 * @param alloc Allocator used for allocating the resulting string.
 * @param file Open file handle to read completely.
 * @returns Result containing the full file contents on success.
 */
static inline ResultOwnedStr file_readall_str(Allocator *alloc, File *file)
{
    ResultU64 tellRes = file_tell(file);
    if (tellRes.error.code)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("file", "file_readall_str",
                ERR_FAILED, "file tell failure"),
        };

    file_rewind(file);
    ResultOwnedStr res = file_read_str(alloc, file, file_size(file));
    file_seek(file, tellRes.value, 0);
    return res;
}

/**
 * Reads the entire file into a newly allocated `HeapBuff`.
 * The caller owns the buffer and must free it with the allocator.
 * @example
 * ```c
 * ResultOwnedBuff res = file_readall_bytes(&allocator, file);
 * if (res.error.code == ERR_OK) {
 *     HeapBuff bytes = res.value;
 *     // Use bytes.bytes / bytes.size
 *     buffer_free(&allocator, &bytes);
 * }
 * ```
 * @param alloc Allocator used to allocate the resulting byte buffer.
 * @param file Open file handle to read completely.
 * @returns Result containing all file bytes on success.
 */
static inline ResultOwnedBuff file_readall_bytes(Allocator *alloc, File *file)
{
    return file_read_bytes(alloc, file, file_size(file));
}

/**
 * Reads the entire file and splits it into individual lines.
 * The resulting list and its contents belong to the caller.
 * @example
 * ```c
 * ResultList res = file_read_lines(&allocator, file);
 * if (res.error.code == ERR_OK) {
 *     List lines = res.value;
 *     // Iterate over lines
 *     list_free(&allocator, &lines);
 * }
 * ```
 * @param alloc Allocator used for intermediate and final allocations.
 * @param file Open file handle to read completely.
 * @returns Result that wraps a `List` of newline-separated strings on success.
 */
static inline ResultList file_read_lines(Allocator *alloc, File *file)
{
    // TODO: Might be wiser to read line per line
    // to prevent out of memory cases
    ResultOwnedStr res = file_readall_str(alloc, file);
    if (res.error.code)
        return (ResultList){
            .error = res.error,
        };

    ResultList split = string_split_lines(alloc, res.value);
    if (split.error.code)
        return (ResultList){
            .error = split.error,
        };

    alloc->free(alloc, res.value);

    return (ResultList){
        .value = split.value,
        .error = X_ERR_OK,
    };
}

/**
 * Writes a single byte to the current position of a file.
 * @param file Open file handle to write into.
 * @param byte Raw byte value to emit.
 * @returns `ERR_OK` on success, otherwise an error describing the failure.
 */
static inline Error file_write_byte(File *file, const i8 byte)
{
    if (!file || !file->_valid)
        return X_ERR_EXT("file", "file_write_byte",
            ERR_INVALID_PARAMETER, "null or invalid file");

    i32 err = _default_file_os_int()->putc(byte, file->_handle);
    if (err == -1) {
        return X_ERR_EXT("file", "file_write_byte",
            ERR_FILE_CANT_WRITE, "write failure");
    }
    return X_ERR_OK;
}

/**
 * Writes a single character to a file.
 * This is a thin wrapper around `file_write_byte` for readability.
 * @param file Open file handle to write into.
 * @param c Character to write; only the low byte is used.
 * @returns `ERR_OK` on success or an error produced by `file_write_byte`.
 */
static inline Error file_write_char(File *file, const i8 c)
{
    return file_write_byte(file, c);
}

/**
 * Writes a null-terminated string to a file.
 * The data may be buffered; call `file_flush` to force an immediate flush if required.
 * @example
 * ```c
 * file_write_str(&file, "This is a test string.");
 * file_flush(&file);
 * ```
 * @param file Open file handle to write into.
 * @param text Null-terminated string to emit.
 * @returns `ERR_OK` on success or the error that interrupted the write.
 */
static inline Error file_write_str(File *file, ConstStr text)
{
    if (!file || !file->_valid || !text)
        return X_ERR_EXT("file", "file_write_str",
            ERR_INVALID_PARAMETER, "null or invalid arg");

    Error err = X_ERR_OK;
    while (*text && err.code == ERR_OK)
    {
        err = file_write_char(file, *text);
        ++text;
    }
    return err;
}

/**
 * Writes a fixed number of bytes to a file.
 * The data may stay in the write buffer until `file_flush` is called.
 * @param file Open file handle to write into.
 * @param bytes Pointer to the bytes that should be written.
 * @param bytesCount Number of bytes to copy from `bytes`.
 * @returns `ERR_OK` on success or the error triggered by the underlying OS call.
 */
static inline Error file_write_bytes(File *file, Buffer buff)
{
    if (!file || !file->_valid || !buff.bytes || buff.size == 0)
        return X_ERR_EXT("file", "file_write_byte",
            ERR_INVALID_PARAMETER, "null or invalid arg");

    i8 *start = buff.bytes;
    const i8 *end = buff.bytes + buff.size + 1;

    Error err = X_ERR_OK;
    while (start != end && err.code == ERR_OK)
    {
        err = file_write_byte(file, *start);
        ++start;
    }
    return err;
}

/**
 * Forces any buffered writes to be committed to the underlying storage.
 * Flushing can impact performance when used too frequently.
 * @example
 * ```c
 * file_write_str(&file, "Hello");
 * file_flush(&file);
 * ```
 * @param file Open file handle whose output buffer should be flushed.
 */
static inline void file_flush(File *file)
{
    if (!file || !file->_valid)
        return;

    _default_file_os_int()->flush(file->_handle);
}

/**
 * Tests whether the file handle is currently positioned at end-of-file.
 * An invalid handle is treated as EOF to ease error handling.
 * @example
 * ```c
 * while (!file_is_eof(&file)) {
 *     // Read data
 * }
 * ```
 * @param file Open file handle to test.
 * @returns Non-zero when EOF was reached or the handle is invalid.
 */
static inline ibool file_is_eof(File *file)
{
    if (!file || !file->_valid)
        return true;

    return _default_file_os_int()->eof(file->_handle);
}

/**
 * Writes the literal string `(null)` to a file.
 * Useful when serializing optional values that are not present.
 * @param file Open file handle to write into.
 * @returns Result of the underlying `file_write_str` call.
 */
static inline Error file_write_null(File *file)
{
    if (!file || !file->_valid)
        return X_ERR_EXT("file", "file_write_null",
            ERR_INVALID_PARAMETER, "null or invalid file");

    return file_write_str(file, "(null)");
}

/**
 * Writes a signed integer to a file using base-10 formatting.
 * @param file Open file handle to write into.
 * @param i Integer value to emit.
 * @returns `ERR_OK` on success or the error encountered while writing.
 */
static inline Error file_write_int(File *file, const i64 i)
{
    if (!file || !file->_valid)
        return X_ERR_EXT("file", "file_write_int",
            ERR_INVALID_PARAMETER, "null or invalid file");

    char buf[20];
    i64 n = i;
    i16 idx = 0;

    Error err = X_ERR_OK;

    if (n == 0)
    {
        err = file_write_char(file, '0');
        return err;
    }

    if (n < 0)
    {
        err = file_write_char(file, '-');
        if (err.code != ERR_OK)
            return err;

        n = -n;
    }

    while (n != 0)
    {
        i16 d = n % 10;
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        err = file_write_char(file, buf[j]);
        if (err.code != ERR_OK)
            return err;
    }
    return err;
}

/**
 * Writes an unsigned integer to a file using base-10 formatting.
 * @param file Open file handle to write into.
 * @param i Unsigned integer value to emit.
 * @returns `ERR_OK` on success or the error produced by the write calls.
 */
static inline Error file_write_uint(File *file, const u64 i)
{
    if (!file || !file->_valid)
        return X_ERR_EXT("file", "file_write_uint",
            ERR_INVALID_PARAMETER, "null or invalid file");

    char buf[20];
    i16 idx = 0;
    u64 n = i;

    Error err = X_ERR_OK;

    if (n == 0)
    {
        err = file_write_char(file, '0');
        return err;
    }

    while (n != 0)
    {
        i16 d = n % 10;
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        err = file_write_char(file, buf[j]);
        if (err.code != ERR_OK)
            return err;
    }
    return err;
}

/**
 * Writes a floating-point value with the specified decimal precision.
 * The fractional component is rounded to the requested number of digits.
 * @param file Open file handle to write into.
 * @param flt Floating-point value to emit.
 * @param precision Number of digits to print after the decimal point. Must be in range 0-18.
 * @returns `ERR_OK` on success or the error returned by the write helpers.
 */
static inline Error file_write_f64(File *file, const f64 flt, const u64 precision)
{
    if (!file || !file->_valid)
        return X_ERR_EXT("file", "file_write_float",
            ERR_INVALID_PARAMETER, "null or invalid file");
        
    if (precision > 19)
        return X_ERR_EXT("file", "file_write_float",
            ERR_INVALID_PARAMETER, "precision > 19");

    f64 d = flt;
    Error err = X_ERR_OK;

    if (d < 0)
    {
        err = file_write_char(file, '-');
        if (err.code != ERR_OK)
            return err;

        d = -d;
    }

    u64 intPart = (u64)d;
    f64 fracPart = d - (f64)intPart;

    err = file_write_uint(file, intPart);
    if (err.code != ERR_OK)
        return err;

    if (precision <= 0)
        return X_ERR_OK;

    err = file_write_char(file, '.');
    if (err.code != ERR_OK)
        return err;

    f64 scale = math_f64_power(10.0, precision - 1);

    fracPart *= scale;
    u64 fracInt = (u64)(fracPart + 0.5); // rounding

    // Ensure leading zeros in fractional part
    ResultU64 divPowRes = math_u64_power_nooverflow(10, precision - 1);
    if (divPowRes.error.code)
        return X_ERR_EXT("file", "file_write_f64",
            ERR_WOULD_OVERFLOW, "integer overflow");

    u64 div = divPowRes.value;
    while (div > fracInt && div > 1)
    {
        err = file_write_char(file, '0');
        if (err.code != ERR_OK)
            return err;

        div /= 10;
    }

    err = file_write_uint(file, fracInt);
    return err;
}

/**
 * Checks whether a file exists by attempting to open it for reading.
 * @param path Null-terminated path to probe.
 * @returns Non-zero when the file can be opened, `false` otherwise.
 */
static inline ibool file_exists(ConstStr path)
{
    if (!path)
        return false;

    ResultFile res = file_open(path, FileOpenModes.READ);
    if (res.error.code != ERR_OK)
        return false;

    file_close(&res.value);
    return true;
}

/**
 * Creates or truncates a file at the specified path using read/write access.
 * @param path Null-terminated path describing the file to be created.
 * @returns Result object containing the new file handle or an error description.
 */
static inline ResultFile file_create(ConstStr path)
{
    if (!path)
        return (ResultFile){
            .error = X_ERR_EXT("file", "file_create",
                ERR_INVALID_PARAMETER, "null path"),
        };

    const char *mode = _file_openmode_to_str(FileOpenModes.TRUNC_READWRITE);
    if (!mode)
        return (ResultFile){
            .error = X_ERR_EXT("file", "file_create",
                ERR_WOULD_NULL_DEREF, "open mode match failure"),
        };

    void *handle = NULL;
    int err = _default_file_os_int()->open(&handle, path, mode);
    if (err)
    {
        Error newErr = X_ERR_EXT("file", "file_create",
            ERR_FILE_CANT_OPEN, "create failure");

        switch (err)
        {
        // ENOENT
        case 2:
            newErr = X_ERR_EXT("file", "file_create",
                ERR_DIR_NOT_FOUND, "parent directory not found");
            break;
        default:
            break;
        }

        return (ResultFile){
            .error = newErr,
        };
    }

    return (ResultFile){
        .value = (File){
            ._handle = handle,
            ._valid = true,
        },
        .error = X_ERR_OK,
    };
}
