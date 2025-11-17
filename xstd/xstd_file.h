#pragma once

#include "xstd_alloc_default.h"
#include "xstd_core.h"
#include "xstd_errcode.h"
#include "xstd_error.h"
#include "xstd_result.h"
#include "xstd_list.h"
#include "xstd_string.h"
#include "xstd_buffer.h"
#include "xstd_writer.h"
#include "xstd_mem.h"
#include "xstd_file_os_int.h"
#include "xstd_file_os_int_default.h"

/**
 * Symbolic value that selects how a file should be opened.
 * Use the predefined members of `EnumFileOpenMode` when calling `file_open`.
 */
typedef ConstStr FileOpenMode;

/**
 * Named constants describing the supported `FileOpenMode` values for `file_open`.
 */
static const struct _file_open_mode
{
    const FileOpenMode READ;
    const FileOpenMode WRITE;
    const FileOpenMode READWRITE;
    const FileOpenMode TRUNC_READWRITE;
    const FileOpenMode APPEND;
} EnumFileOpenMode = {
    .READ = "rb",
    .WRITE = "wb",
    .READWRITE = "r+b",
    .TRUNC_READWRITE = "w+b",
    .APPEND = "a+b",
};

typedef struct _file
{
    void *_handle;
    Bool _valid;
} File;

result_define(File, File);

/**
 * Opens a file using the default operating system backend.
 * @example
 * ```c
 * ResultFile res = file_open("test.txt", EnumFileOpenMode.READ);
 * if (res.error.code == ERR_OK) {
 *     File file = res.value;
 *     file_close(&file);
 * }
 * ```
 * @param path Null-terminated path to the file to open.
 * @param mode One of the values exposed on `EnumFileOpenMode`.
 * @returns Result object containing the opened file or an error description.
 */
static inline result_type(File) file_open(ConstStr path, FileOpenMode mode)
{
    if (!path)
        return result_err(File, X_ERR_EXT("file", "file_open", ERR_INVALID_PARAMETER, "null path"));

    if (!mode)
        return result_err(File, X_ERR_EXT("file", "file_open", ERR_INVALID_PARAMETER, "null mode"));

    void *f;

    int err = _default_file_os_int()->open(&f, path, mode);

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

        return result_err(File, newErr);
    }

    File fl = {
        ._handle = f,
        ._valid = 1,
    };
    return result_ok(File, fl);
}

/**
 * Closes a file handle obtained from `file_open`.
 * @example
 * ```c
 * ResultFile res = file_open("example.txt", EnumFileOpenMode.READ);
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
 * File file = file_open("example.txt", EnumFileOpenMode.READ).value;
 * u64 size = file_size(&file);
 * ```
 * @param file Open file handle whose size should be queried.
 * @returns File length in bytes, or `0` when the handle is invalid.
 */
static inline u64 file_size(File *file)
{
    if (!file || !file->_valid)
        return 0;

    const _FileOsInterface* foi = _default_file_os_int();
    return foi->size(file->_handle);
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
static inline Bool file_is_eof(File *file)
{
    if (!file || !file->_valid)
        return true;

    return _default_file_os_int()->eof(file->_handle);
}

static inline u64 _file_read_internal(File *f, i8 *buff, const u64 nBytes, Bool terminate)
{
    if (!f || !nBytes || !buff)
        return 0;

    const _FileOsInterface* foi = _default_file_os_int();
    u64 readPos = 0;

    if (foi->read)
    {
        while (readPos < nBytes)
        {
            u64 toRead = nBytes - readPos;
            u64 chunk = foi->read(f->_handle, buff + readPos, toRead);
            if (chunk == 0)
                break;

            readPos += chunk;

            if (chunk < toRead)
                break;
        }
    }
    else
    {
        while (readPos < nBytes)
        {
            int read = foi->getc(f->_handle);
            if (read < 0)
                break;

            buff[readPos] = (i8)read;
            ++readPos;
        }
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
static inline result_type(U64) file_tell(File *file)
{
    if (!file || !file->_valid)
        return result_err(U64, X_ERR_EXT("file", "file_tell", ERR_INVALID_PARAMETER, "null or invalid file"));

    const long pos = _default_file_os_int()->tell(file->_handle);
    if (pos < 0)
        return result_err(U64, X_ERR_EXT("file", "file_tell", ERR_FAILED, "tell failure"));

    return result_ok(U64, (u64)pos);
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
static inline result_type(OwnedBuff) file_read_bytes(Allocator *a, File *file, const u64 nBytes)
{
    if (!a || !file || !file->_valid)
        return result_err(OwnedBuff, X_ERR_EXT("file", "file_read_bytes", ERR_INVALID_PARAMETER, "null arg"));

    if (nBytes == 0) {
        HeapBuff hb = {.bytes = NULL,.size = 0};
        return result_ok(OwnedBuff, hb);
    }

    i8 *newBuff = (i8*)a->alloc(a, nBytes);

    if (!newBuff)
        return result_err(OwnedBuff, X_ERR_EXT("file", "file_read_bytes", ERR_OUT_OF_MEMORY, "alloc failure"));

    u64 readSize = _file_read_internal(file, newBuff, nBytes, false);

    if (readSize == 0)
    {
        if (file_is_eof(file)) {
            HeapBuff hb = {.bytes = newBuff, .size = 0};
            return result_ok(OwnedBuff, hb);
        }

        a->free(a, newBuff);
        return result_err(OwnedBuff, X_ERR_EXT("file", "file_read_bytes", ERR_FILE_CANT_READ, "read size mismatch"));
    }

    HeapBuff hb = {.bytes = newBuff, .size = readSize};
    return result_ok(OwnedBuff, hb);
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
static inline result_type(OwnedStr) file_read_str(Allocator *a, File *file, const u64 nBytes)
{
    if (!a || !file || !file->_valid)
        return result_err(OwnedStr, X_ERR_EXT("file", "file_read_str", ERR_INVALID_PARAMETER, "null arg"));

    if (nBytes == 0)
    {
        HeapStr empty = (HeapStr)a->alloc(a, 1);
        if (!empty)
            return result_err(OwnedStr, X_ERR_EXT("file", "file_read_str", ERR_OUT_OF_MEMORY, "alloc failure"));

        empty[0] = 0;
        return result_ok(OwnedStr, empty);
    }

    HeapStr newStr = (HeapStr)a->alloc(a, nBytes + 1);

    if (!newStr)
        return result_err(OwnedStr, X_ERR_EXT("file", "file_read_str", ERR_OUT_OF_MEMORY, "alloc failure"));

    u64 readSize = _file_read_internal(file, (i8*)newStr, nBytes, true);

    if (readSize == 0)
    {
        if (file_is_eof(file))
        {
            newStr[0] = 0;
            return result_ok(OwnedStr, newStr);
        }

        a->free(a, newStr);
        return result_err(OwnedStr, X_ERR_EXT("file", "file_read_str", ERR_FILE_CANT_READ, "read size mismatch"));
    }

    return result_ok(OwnedStr, newStr);
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

    u64 readSize = _file_read_internal(file, (i8*)newStr, nBytes, true);

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
static inline result_type(OwnedStr) file_readall_str(Allocator *alloc, File *file)
{
    result_type(U64) tellRes = file_tell(file);
    if (tellRes.isErr)
        return result_err(OwnedStr, X_ERR_EXT("file", "file_readall_str", ERR_FAILED, "file tell failure"));

    file_rewind(file);
    u64 totalSize = file_size(file);
    result_type(OwnedStr) res = file_read_str(alloc, file, totalSize);
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
static inline result_type(OwnedBuff) file_readall_bytes(Allocator *alloc, File *file)
{
    return file_read_bytes(alloc, file, file_size(file));
}

static inline Error _file_append_line(Allocator *alloc, List *lines, Writer *w)
{
    if (!lines || !alloc || !w)
        return X_ERR_EXT("file", "_file_append_line",
            ERR_INVALID_PARAMETER, "null arg");

    result_type(OwnedBuff) dataRes = growbuffwriter_data(w);
    if (dataRes.isErr)
        return dataRes.err;

    Buffer segment = dataRes.value;

    u64 lineLen = segment.size;
    OwnedStr line = (OwnedStr)alloc->alloc(alloc, lineLen + 1);
    if (!line)
        return X_ERR_EXT("file", "_file_append_line",
            ERR_OUT_OF_MEMORY, "alloc failure");

    if (lineLen && segment.bytes)
        mem_copy(line, segment.bytes, lineLen);
    line[lineLen] = 0;

    String stored = line;
    Error pushErr = list_push_result(lines, &stored);
    if (pushErr.code != ERR_OK)
    {
        alloc->free(alloc, line);
        return pushErr;
    }

    Error resetErr = growbuffwriter_reset(w, 128);
    if (resetErr.code != ERR_OK)
    {
        alloc->free(alloc, line);
        return resetErr;
    }

    return X_ERR_OK;
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
static inline result_type(List) file_read_lines(Allocator *a, File *file)
{
    if (!a || !file || !file->_valid)
        return result_err(List, X_ERR_EXT("file", "file_read_lines", ERR_INVALID_PARAMETER, "null allocator or file"));

    result_type(U64) tellRes = file_tell(file);
    if (tellRes.isErr)
        return result_err(List, X_ERR_EXT("file", "file_readall_str", ERR_FAILED, "file tell failure"));

    file_rewind(file);

    result_type(List) listRes = ListInitT(String, a);
    if (listRes.isErr)
        return listRes;

    List lines = listRes.value;

    result_type(Writer) writerRes = growbuffwriter_init(*a, 128);
    if (writerRes.isErr)
    {
        list_deinit(&lines);
        return result_err(List, writerRes.err);
    }

    Writer writer = writerRes.value;
    const _FileOsInterface *foi = _default_file_os_int();
    int pending = -2;
    Error err = X_ERR_OK;

    while (1)
    {
        int ch;
        if (pending != -2)
        {
            ch = pending;
            pending = -2;
        }
        else
        {
            ch = foi->getc(file->_handle);
        }

        if (ch == -1)
            break;

        if (ch == '\n')
        {
            err = _file_append_line(a, &lines, &writer);
            if (err.code != ERR_OK)
                goto _file_read_lines_cleanup;
            continue;
        }

        if (ch == '\r')
        {
            int next = foi->getc(file->_handle);
            if (next != '\n' && next != -1)
                pending = next;

            err = _file_append_line(a, &lines, &writer);
            if (err.code != ERR_OK)
                goto _file_read_lines_cleanup;
            continue;
        }

        err = writer_write_byte(&writer, (i8)ch);
        if (err.code != ERR_OK)
            goto _file_read_lines_cleanup;
    }

    err = _file_append_line(a, &lines, &writer);
    if (err.code != ERR_OK)
        goto _file_read_lines_cleanup;

    growbuffwriter_deinit(&writer);
    file_seek(file, tellRes.value, 0);

    return result_ok(List, lines);

_file_read_lines_cleanup:
    growbuffwriter_deinit(&writer);

    list_free_items(a, &lines);
    list_deinit(&lines);

    file_seek(file, tellRes.value, 0);

    if (err.code == ERR_OK)
        err = X_ERR_EXT("file", "file_read_lines",
            ERR_FAILED, "read failure");

    return result_err(List, err);
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

    const _FileOsInterface *foi = _default_file_os_int();

    if (foi->write)
    {
        u64 totalWritten = 0;
        while (totalWritten < buff.size)
        {
            u64 remaining = buff.size - totalWritten;
            u64 written = foi->write(file->_handle, buff.bytes + totalWritten, remaining);
            if (written == 0)
                break;

            totalWritten += written;

            if (written < remaining)
                break;
        }

        if (totalWritten != buff.size)
            return X_ERR_EXT("file", "file_write_bytes",
                ERR_FILE_CANT_WRITE, "write failure");

        return X_ERR_OK;
    }

    i8 *start = buff.bytes;
    const i8 *end = buff.bytes + buff.size;

    Error err = X_ERR_OK;
    while (start != end && err.code == ERR_OK)
    {
        err = file_write_byte(file, *start);
        ++start;
    }
    return err;
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

    u64 len = string_size(text);
    if (len == 0)
        return X_ERR_OK;

    Buffer buff = (Buffer){
        .bytes = (i8*)text,
        .size = len,
    };

    return file_write_bytes(file, buff);
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
    result_type(U64) divPowRes = math_u64_power_nooverflow(10, precision - 1);
    if (divPowRes.isErr)
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
static inline Bool file_exists(ConstStr path)
{
    if (!path)
        return false;

    result_type(File) res = file_open(path, EnumFileOpenMode.READ);
    if (res.isErr)
        return false;

    file_close(&res.value);
    return true;
}

/**
 * Creates or truncates a file at the specified path using read/write access.
 * @param path Null-terminated path describing the file to be created.
 * @returns Result object containing the new file handle or an error description.
 */
static inline result_type(File) file_create(ConstStr path)
{
    if (!path)
        return result_err(File, X_ERR_EXT("file", "file_create", ERR_INVALID_PARAMETER, "null path"));

    void *handle = NULL;
    int err = _default_file_os_int()->open(&handle, path, EnumFileOpenMode.TRUNC_READWRITE);
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

        return result_err(File, newErr);
    }

    File f = {
        ._handle = handle,
        ._valid = true,
    };
    return result_ok(File, f);
}

static inline Error _file_writer_write(Writer* w, i8 byte)
{
    if (!w || !w->_internalState)
        return X_ERR_EXT("file", "_file_writer_write",
            ERR_INVALID_PARAMETER, "null or invalid arg");
    
    File* f = (void*)w->_internalState;
    return file_write_byte(f, byte);
}

static inline void file_writer_deinit(Writer* w)
{
    if (!w)
        return;
    
    if (w->_internalState)
        w->_internalState = NULL;
}

static inline result_type(Writer) file_writer_init(File* f)
{
    if (!f || !f->_handle || !f->_valid)
        return result_err(Writer, X_ERR_EXT("file", "file_writer_init", ERR_INVALID_PARAMETER, "null or invalid arg"));

    Writer w = {
        ._internalState = (void*)f,
        .write = &_file_writer_write,
        .deinit = &file_writer_deinit,
    };
    // TODO: revamp Writer interface to allow for many-bytes writes
    return result_ok(Writer, w);
}
