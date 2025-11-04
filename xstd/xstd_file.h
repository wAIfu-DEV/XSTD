#pragma once

#include "xstd_core.h"
#include "xstd_error.h"
#include "xstd_result.h"
#include "xstd_list.h"
#include "xstd_string.h"
#include "xstd_buffer.h"
#include "xstd_file_os_int.h"

typedef u8 FileOpenMode;

#define FILE_OPENMODE_READ 0
#define FILE_OPENMODE_WRITE 1
#define FILE_OPENMODE_READWRITE 2
#define FILE_OPENMODE_APPEND 3

const struct _file_open_mode
{
    const FileOpenMode READ;
    const FileOpenMode WRITE;
    const FileOpenMode READWRITE;
    const FileOpenMode APPEND;
} FileOpenModes = {
    .READ = FILE_OPENMODE_READ,
    .WRITE = FILE_OPENMODE_WRITE,
    .READWRITE = FILE_OPENMODE_READWRITE,
    .APPEND = FILE_OPENMODE_APPEND,
};

typedef struct _file
{
    void *_handle;
    ibool _valid;
} File;

typedef struct _result_file
{
    File value;
    Error error;
} ResultFile;

const char *_file_openmode_to_str(const FileOpenMode mode)
{
    switch (mode)
    {
    case FILE_OPENMODE_READ:
        return "rb";
    case FILE_OPENMODE_WRITE:
        return "wb";
    case FILE_OPENMODE_READWRITE:
        return "w+b";
    case FILE_OPENMODE_APPEND:
        return "a+b";
    default:
        return NULL;
    }
}

/**
 * @brief Opens file at `path`. Opened files should be ultimately closed.
 *
 * ```c
 * ResultFile res = file_open("test.txt", FileOpenModes.READ, FileOpenFormats.TEXT);
 * if (res.error.code) // Error!
 * File* file = res.value;
 * // Do file stuff
 * file_close(file);
 * ```
 * @param path
 * @param mode
 * @param format
 * @return ResultFile
 */
ResultFile file_open(ConstStr path, const FileOpenMode mode)
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

    int err = __file_os_int.open(&f, path, openArg);

    if (err)
    {
        Error newErr = X_ERR_EXT("file", "file_open", ERR_FILE_CANT_OPEN, "open failure");
        switch (err)
        {
        // ENOENT
        case 2:
            newErr = X_ERR_EXT("file", "file_open", ERR_FILE_NOT_FOUND, "file not found");
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
 * @brief Closes the file. Once a file is closed, it cannot be reused.
 *
 * ```c
 * ResultFile res = file_open("test.txt", FileOpenModes.READ, FileOpenFormats.TEXT);
 * if (res.error.code) // Error!
 * File* file = res.value;
 * // Do file stuff
 * file_close(file);
 * ```
 * @param file
 */
void file_close(File *file)
{
    if (!file)
        return;

    if (!file->_valid)
        return;

    __file_os_int.close(file->_handle);
    file->_valid = false;
}

/**
 * @brief Returns the size of the contents of the file.
 *
 * ```c
 * u64 fileSize = file_size(file);
 * ```
 * @param f
 * @return u64
 */
u64 file_size(File *file)
{
    if (!file || !file->_valid)
        return 0;

    const int seekEnd = 2;
    const int seekSet = 0;

    long ogTell = __file_os_int.tell(file->_handle);

    __file_os_int.seek(file->_handle, 0, seekEnd);
    long fileSize = __file_os_int.tell(file->_handle);

    __file_os_int.seek(file->_handle, ogTell, seekSet);
    return (u64)fileSize;
}

u64 __file_read_internal(File *f, i8 *buff, const u64 nBytes, ibool terminate)
{
    if (!f || !nBytes || !buff)
        return 0;

    u64 readPos = 0;

    int read;
    while (!__file_os_int.eof(f->_handle) && readPos < nBytes)
    {
        read = __file_os_int.getc(f->_handle);
        buff[readPos] = (i8)read;
        ++readPos;
    }

    if (terminate)
        buff[readPos] = (i8)0;

    return readPos;
}

/**
 * @brief Reads `nBytes` (or less if EOF) of the file into a newly allocated HeapBuff.
 *
 * Memory is owned by the caller and should be freed.
 *
 * ```c
 * // Read file in chunks
 * while(!file_is_eof(file))
 * {
 *     ResultOwnedBuff res = file_read_bytes(&allocator, file, 256);
 *     if (res.error.code) // Error!
 *     HeapBuff fileChunk = res.value;
 *     // Do stuff
 *     buffer_free(&allocator, &fileChunk);
 * }
 * ```
 * @param alloc
 * @param file
 * @param nBytes
 * @return ResultOwnedBuff
 */
ResultOwnedBuff file_read_bytes(Allocator *alloc, File *file, const u64 nBytes)
{
    if (!alloc || !file || !file->_valid)
        return (ResultOwnedBuff){
            .value = (HeapBuff){.bytes = NULL, .size = 0},
            .error = X_ERR_EXT("file", "file_read_bytes", ERR_INVALID_PARAMETER, "null arg"),
        };

    i8 *new = alloc->alloc(alloc, nBytes);

    if (!new)
        return (ResultOwnedBuff){
            .value = (HeapBuff){.bytes = NULL, .size = 0},
            .error = X_ERR_EXT("file", "file_read_bytes", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    u64 readSize = __file_read_internal(file, new, nBytes, false);

    if (readSize == 0)
    {
        alloc->free(alloc, new);
        return (ResultOwnedBuff){
            .value = (HeapBuff){.bytes = NULL, .size = 0},
            .error = X_ERR_EXT("file", "file_read_bytes", ERR_FILE_CANT_READ, "read size mismatch"),
        };
    }

    return (ResultOwnedBuff){
        .value = (HeapBuff){.bytes = new, .size = readSize},
        .error = X_ERR_OK,
    };
}

/**
 * @brief Reads `nBytes` (or less if EOF) of the file into a newly allocated OwnedStr.
 *
 * Memory is owned by the caller and should be freed.
 *
 * ```c
 * // Read file in chunks
 * while(!file_is_eof(file))
 * {
 *     ResultOwnedStr res = file_read_str(&allocator, file, 256);
 *     if (res.error.code) // Error!
 *     OwnedStr fileChunk = res.value;
 *     // Do string stuff
 *     allocator.free(&allocator, fileChunk);
 * }
 * ```
 * @param alloc
 * @param file
 * @param nBytes
 * @return ResultOwnedStr
 */
ResultOwnedStr file_read_str(Allocator *alloc, File *file, const u64 nBytes)
{
    if (!alloc || !file || !file->_valid)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("file", "file_read_bytes", ERR_INVALID_PARAMETER, "null arg"),
        };

    HeapStr newStr = alloc->alloc(alloc, nBytes + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("file", "file_read_bytes", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    u64 readSize = __file_read_internal(file, newStr, nBytes, true);

    if (readSize == 0)
    {
        alloc->free(alloc, newStr);
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("file", "file_read_bytes", ERR_FILE_CANT_READ, "read size mismatch"),
        };
    }

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Reads `nBytes` (or less if EOF) of the file into a newly allocated HeapStr.
 *
 * Memory is owned by the caller and should be freed.
 *
 * ```c
 * // Read file in chunks
 * while(!file_is_eof(file))
 * {
 *     OwnedStr fileChunk = file_read_str_unsafe(&allocator, file, 256);
 *     if (!fileChunk) // Error!
 *     // Do string stuff
 *     allocator.free(&allocator, fileChunk);
 * }
 * ```
 * @param alloc
 * @param file
 * @param nBytes
 * @return OwnedStr
 */
OwnedStr file_read_str_unsafe(Allocator *alloc, File *file, u64 nBytes)
{
    HeapStr newStr = alloc->alloc(alloc, nBytes + 1);

    if (!newStr)
        return NULL;

    u64 readSize = __file_read_internal(file, newStr, nBytes, true);

    if (readSize == 0)
    {
        alloc->free(alloc, newStr);
        return NULL;
    }
    return newStr;
}

/**
 * @brief Reads `nBytes` (or less if EOF) of the file into a newly allocated HeapBuff.
 *
 * Memory is owned by the caller and should be freed.
 *
 * ```c
 * // Read file in chunks
 * while(!file_is_eof(file))
 * {
 *     HeapBuff fileChunk = file_read_bytes_unsafe(&allocator, file, 256);
 *     if (!fileChunk) // Error!
 *     // Do string stuff
 *     buffer_free(&fileChunk);
 * }
 * ```
 * @param alloc
 * @param file
 * @param nBytes
 * @return HeapBuff
 */
HeapBuff file_read_bytes_unsafe(Allocator *alloc, File *file, u64 nBytes)
{
    i8 *new = alloc->alloc(alloc, nBytes);

    if (!new)
        return (HeapBuff){.bytes = NULL, .size = 0};

    u64 readSize = __file_read_internal(file, new, nBytes, false);

    if (readSize == 0)
    {
        alloc->free(alloc, new);
        return (HeapBuff){.bytes = NULL, .size = 0};
    }

    return (HeapBuff){.bytes = new, .size = readSize};
}

/**
 * @brief Reads the entirety of the file into a newly allocated OwnedStr.
 *
 * Memory is owned by the caller and should be freed.
 *
 * ```c
 * ResultOwnedStr res = file_readall_str(&allocator, file);
 * if (res.error.code) // Error!
 * OwnedStr fileContents = res.value;
 * // Do string stuff
 * allocator.free(&allocator, fileContents);
 * ```
 * @param alloc
 * @param file
 * @param nBytes
 * @return ResultOwnedStr
 */
ResultOwnedStr file_readall_str(Allocator *alloc, File *file)
{
    return file_read_str(alloc, file, file_size(file));
}

/**
 * @brief Reads the entirety of the file into a newly allocated HeapBuff.
 *
 * Memory is owned by the caller and should be freed.
 *
 * ```c
 * ResultOwnedBuff res = file_readall_bytes(&allocator, file);
 * if (res.error.code) // Error!
 * HeapBuff fileContents = res.value;
 * // Do stuff
 * buff_free(&allocator, &fileContents);
 * ```
 * @param alloc
 * @param file
 * @param nBytes
 * @return ResultOwnedBuff
 */
ResultOwnedBuff file_readall_bytes(Allocator *alloc, File *file)
{
    return file_read_bytes(alloc, file, file_size(file));
}

/**
 * @brief Reads the entirety of the file and splits it into lines.
 *
 * Memory is owned by the caller and should be freed.
 *
 * ```c
 * ResultList res = file_read_lines(&allocator, file);
 * if (res.error.code) // Error!
 * List fileLines = res.value;
 * // Do string stuff
 * allocator.free(&allocator, fileContents);
 * ```
 * @param alloc
 * @param file
 * @return ResultList
 */
ResultList file_read_lines(Allocator *alloc, File *file)
{
    // TODO: Might be wiser to read line per line
    // to prevent out of memory cases
    ResultOwnedStr res = file_readall_str(alloc, file);
    if (res.error.code)
        return (ResultList){
            .value = {0},
            .error = res.error,
        };

    ResultList split = string_split_lines(alloc, res.value);
    if (split.error.code)
        return (ResultList){
            .value = {0},
            .error = split.error,
        };

    alloc->free(alloc, res.value);

    return (ResultList){
        .value = split.value,
        .error = X_ERR_OK,
    };
}

void file_write_byte(File *file, const i8 byte)
{
    if (!file || !file->_valid)
        return;

    FILE *f = file->_handle;
    __file_os_int.putc(byte, f);
}

void file_write_char(File *file, const i8 c)
{
    file_write_byte(file, c);
}

/**
 * @brief Writes a string to a file.
 *
 * Written data may be buffered, use `file_flush()` if you require an immediate write.
 *
 * ```c
 * ConstStr myStr = "This is a test string.";
 * file_write_str(file, myStr);
 * // Wrote "This is a test string."
 * ```
 * @param file
 * @param text
 */
void file_write_str(File *file, ConstStr text)
{
    if (!file || !file->_valid || !text)
        return;

    while (*text)
    {
        file_write_char(file, *text);
        ++text;
    }
}

/**
 * @brief Writes a bytes buffer to a file.
 *
 * Written data may be buffered, use `file_flush()` if you require an immediate write.
 *
 * ```c
 * ConstStr myStr = "This is a test string.";
 * file_write_bytes(file, myStr, 4);
 * // Wrote "This"
 * ```
 * @param file
 * @param text
 */
void file_write_bytes(File *file, const i8 *bytes, u64 bytesCount)
{
    if (!file || !file->_valid || !bytes || bytesCount == 0)
        return;

    const i8 *end = bytes + bytesCount + 1;

    while (bytes != end)
    {
        file_write_byte(file, *bytes);
        ++bytes;
    }
}

/**
 * @brief Flushes the write buffer, allows immediate writing of buffered data,
 * but may come at the cost of performances.
 *
 * ```c
 * ConstStr myStr = "This is a test string.";
 * file_write_str(file, myStr);
 * file_flush(file);
 * // Wrote "This is a test string."
 * ```
 * @param file
 */
void file_flush(File *file)
{
    if (!file || !file->_valid)
        return;

    __file_os_int.flush(file->_handle);
}

/**
 * @brief Returns !0 (true) if file has hit EOF (end of file), returns 0 (false) otherwise.
 *
 * ```c
 * // Read file in chunks
 * while(!file_is_eof(file))
 * {
 *     OwnedStr fileChunk = file_read_str_unsafe(&c_allocator, file, 256);
 *     if (!fileChunk) // Error!
 *     // Do string stuff
 *     c_allocator.free(&c_allocator, fileChunk);
 * }
 * ```
 * @param file
 * @return ibool
 */
ibool file_is_eof(File *file)
{
    if (!file || !file->_valid)
        return true;

    return __file_os_int.eof(file->_handle);
}

void file_write_null(File *file)
{
    if (!file || !file->_valid)
        return;

    file_write_str(file, "(null)");
}

void file_write_int(File *file, const i64 i)
{
    if (!file || !file->_valid)
        return;

    char buf[20];
    i64 n = i;
    i16 idx = 0;

    if (n == 0)
    {
        file_write_char(file, '0');
        return;
    }

    if (n < 0)
    {
        file_write_char(file, '-');
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
        file_write_char(file, buf[j]);
    }
}

void file_write_uint(File *file, const u64 i)
{
    if (!file || !file->_valid)
        return;

    char buf[20];
    i16 idx = 0;
    u64 n = i;

    if (n == 0)
    {
        file_write_char(file, '0');
        return;
    }

    while (n != 0)
    {
        i16 d = n % 10;
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        file_write_char(file, buf[j]);
    }
}

void file_write_f64(File *file, const f64 flt, const u64 precision)
{
    if (!file || !file->_valid)
        return;

    f64 d = flt;

    if (d < 0)
    {
        file_write_char(file, '-');
        d = -d;
    }

    u64 intPart = (u64)d;
    f64 fracPart = d - (f64)intPart;

    file_write_uint(file, intPart);

    if (precision <= 0)
        return;

    file_write_char(file, '.');

    f64 scale = 1.0;
    for (u64 i = 0; i < precision; ++i)
        scale *= 10.0;

    fracPart *= scale;
    u64 fracInt = (u64)(fracPart + 0.5); // rounding

    // Ensure leading zeros in fractional part
    i64 div = 1;
    for (int i = 1; i < precision; ++i)
        div *= 10;

    while (div > fracInt && div > 1)
    {
        file_write_char(file, '0');
        div /= 10;
    }

    file_write_uint(file, fracInt);
}

// TODO: file_exists, file_create, file_seek, file_tell, file_rewind
