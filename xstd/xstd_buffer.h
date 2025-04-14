#pragma once

#include "xstd_coretypes.h"
#include "xstd_err.h"
#include "xstd_result.h"
#include "xstd_alloc.h"

/**
 * @brief Compares the contents of two buffers and returns !0 (true) if both buffers are similar,
 * or 0 (false) if they aren't.
 *
 * ```c
 * ConstStr a = "Example string";
 * ConstStr b = "Example string2"
 * ConstStr c = "Example string";
 * Buffer ba = (Buffer){ .bytes = a, .size = string_size(a) + 1 };
 * Buffer bb = (Buffer){ .bytes = b, .size = string_size(b) + 1 };
 * Buffer bc = (Buffer){ .bytes = c, .size = string_size(c) + 1 };
 * string_equals(ba, bb); // 0 (false)
 * string_equals(ba, bc); // !0 (true)
 * ```
 * @param a
 * @param b
 * @return ibool
 */
ibool buffer_equals(ConstBuff a, ConstBuff b)
{
    if (a.size != b.size)
        return false;

    if (a.size == 0)
        return true;

    if (a.bytes == b.bytes)
        return true;

    if (!a.bytes || !b.bytes)
        return false;

    const i8 *va = (const i8 *)a.bytes;
    const i8 *vb = (const i8 *)b.bytes;

    u64 i = 0;
    while (i < a.size - 1 && va[i] == vb[i])
    {
        ++i;
    }
    return !(va[i] - vb[i]);
}

/**
 * @brief Allocates a buffer of size `size`
 * on the heap. The contents of the buffer is filled with `fill`
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ResultOwnedBuff myBuff = buffer_alloc(&c_allocator, 3, 'w');
 * if (myBuff.error) // Error!
 * // myBuff.value.bytes == ['w', 'w', 'w']
 * // myBuff.value.size == 3
 * ```
 * @param alloc
 * @param size
 * @param fill
 * @return ResultOwnedBuff
 */
ResultOwnedBuff buffer_alloc(Allocator *alloc, u64 size, i8 fill)
{
    i8 *bytes = alloc->alloc(alloc, size);

    if (!bytes)
        return (ResultOwnedBuff){
            .value = (HeapBuff){.bytes = NULL, .size = 0},
            .error = ERR_OUT_OF_MEMORY,
        };

    u64 i = size;

    while (i--)
    {
        bytes[i] = fill;
    }

    return (ResultOwnedBuff){
        .value = (HeapBuff){
            .bytes = bytes,
            .size = size,
        },
        .error = ERR_OK,
    };
}

/**
 * @brief Frees the memory allocated for the buffer. Do not reuse the buffer after
 * call to this function.
 *
 * Do not use this function with `Buffer` or `ConstBuff`, make sure `buff` is truly
 * a `HeapBuff` allocated using the passed allocator.
 *
 * @param buff
 */
void buffer_free(Allocator *alloc, HeapBuff *buff)
{
    if (!alloc || !buff)
        return;

    if (!buff->bytes)
        return;

    alloc->free(alloc, buff->bytes);

    buff->bytes = NULL;
    buff->size = 0;
}

/**
 * @brief Copies the contents of buffer `source` to buffer `destination`. Checks if `destination`
 * is big enough to receive `source`, if not returns `ERR_WOULD_OVERFLOW`.
 *
 * Resizes the `destination` buffer after copying from `source`, meaning the
 * destination buffer may get smaller.
 * If you don't want this, use `buffer_copy_n(src, dst, src.size)`
 *
 * ```c
 * ConstStr source =     "Example string";
 * HeapStr dest = ConstToHeapStr("____________________");
 *
 * ConstBuff srcBuff = (ConstBuff){ .bytes = source, .size = string_size(source) + 1 };
 * HeapBuff destBuff = (HeapBuff){ .bytes = dest, .size = string_size(dest) + 1 };
 *
 * Error err = buffer_copy(srcBuff, destBuff);
 * if (err) // Error!
 * // destBuff.bytes == "Example string______"
 * // destBuff.size == sizeof("Example string")
 * ```
 * @param source
 * @param destination buffer of size >= source size
 * @return Error
 */
Error buffer_copy(ConstBuff source, Buffer destination)
{
    if (!source.bytes || !destination.bytes)
        return ERR_INVALID_PARAMETER;

    if (destination.size < source.size)
        return ERR_WOULD_OVERFLOW;

    const i8 *s = (const i8 *)source.bytes;
    i8 *d = (i8 *)destination.bytes;

    u64 i = 0;
    while (i < source.size)
    {
        d[i] = s[i];
        ++i;
    }

    destination.size = source.size;
    return ERR_OK;
}

/**
 * @brief Copies `n` bytes from `source` to `destination`. Checks if the `destination`
 * is big enough to receive n bytes, if not returns `ERR_WOULD_OVERFLOW`.
 *
 * ```c
 * ConstStr source = "Example string";
 * HeapStr dest = ConstToHeapStr("______________");
 *
 * ConstBuff srcBuff = (ConstBuff){ .bytes = source, .size = string_size(source) + 1 };
 * HeapBuff destBuff = (HeapBuff){ .bytes = dest, .size = string_size(dest) + 1 };
 *
 * Error err = buffer_copy_n(srcBuff, destBuff, 5);
 * if (err) // Error!
 * // destBuff.bytes == "Examp_________"
 * ```
 * @param source
 * @param destination buffer of size >= n
 * @return Error
 */
Error buffer_copy_n(ConstBuff source, Buffer destination, u64 n)
{
    if (!source.bytes || !destination.bytes)
        return ERR_INVALID_PARAMETER;

    if (source.size < n)
        return ERR_WOULD_OVERFLOW;

    if (destination.size < n)
        return ERR_WOULD_OVERFLOW;

    const i8 *s = (const i8 *)source.bytes;
    i8 *d = (i8 *)destination.bytes;

    u64 i = 0;
    while (i < n)
    {
        d[i] = s[i];
        ++i;
    }
    return ERR_OK;
}

/**
 * @brief Unsafe version of `buffer_copy`.
 * Copies a string from `source` to `destination`. Does not check for overflow,
 * and will cause undefined behavior if `destination` is smaller than `source`.
 *
 * Resizes the `destination` buffer after copying from `source`, meaning the
 * destination buffer may get smaller.
 * If you don't want this, use `buffer_copy_n_unsafe(src, dst, src.size)`
 *
 * ```c
 * ConstStr source =     "Example string";
 * String dest = ConstToStr("__________________");
 *
 * ConstBuff srcBuff = (ConstBuff){ .bytes = source, .size = string_size(source) + 1 };
 * HeapBuff destBuff = (HeapBuff){ .bytes = dest, .size = string_size(dest) + 1 };
 *
 * buffer_copy_unsafe(srcBuff, destBuff);
 * // destBuff.bytes == "Example string____"
 * // destBuff.size == sizeof("Example string")
 * ```
 * @param source
 * @param destination buffer of size >= source.size
 */
void buffer_copy_unsafe(ConstBuff source, Buffer destination)
{
    const i8 *s = (const i8 *)source.bytes;
    i8 *d = (i8 *)destination.bytes;

    u64 i = 0;
    while (i < source.size)
    {
        d[i] = s[i];
        ++i;
    }
    destination.size = source.size;
}

/**
 * @brief Unsafe version of `buffer_copy_n`.
 * Copies `n` bytes from `source` to `destination`. Does not check for overflow,
 * and will cause undefined behavior if `destination` is smaller than `n` or if
 * `source` is smaller than `n`.
 *
 * ```c
 * ConstStr source = "Example string";
 * String dest = ConstToStr("______________");
 *
 * ConstBuff srcBuff = (ConstBuff){ .bytes = source, .size = string_size(source) + 1 };
 * HeapBuff destBuff = (HeapBuff){ .bytes = dest, .size = string_size(dest) + 1 };
 *
 * buffer_copy_n_unsafe(srcBuff, destBuff, 5);
 * // destBuff.bytes == "Examp_________"
 * ```
 * @param source buffer of size >= n
 * @param destination buffer of size >= n
 * @param n number of bytes to copy
 */
void buffer_copy_n_unsafe(ConstBuff source, Buffer destination, u64 n)
{
    const i8 *s = (const i8 *)source.bytes;
    i8 *d = (i8 *)destination.bytes;

    u64 i = 0;
    while (i < n)
    {
        d[i] = s[i];
        ++i;
    }
}

/**
 * @brief Allocates and creates a copy of `source` on the heap.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr source = "Example string";
 * ConstBuff srcBuff = (ConstBuff){ .bytes = source, .size = string_size(source) + 1 };
 *
 * ResultOwnedBuff newBuff = buff_dupe(&c_allocator, srcBuff);
 * if (newBuff.error) // Error!
 * // newBuff.value.bytes == "Example string"
 * // newBuff.value.size == sizeof("Example string")
 * ```
 * @param alloc
 * @param source
 * @return String
 */
ResultOwnedBuff buffer_dupe(Allocator *alloc, ConstBuff source)
{
    if (source.size == 0)
        return (ResultOwnedBuff){
            .value = {.bytes = NULL, .size = 0},
            .error = ERR_OK,
        };

    if (!source.bytes)
        return (ResultOwnedBuff){
            .value = {.bytes = NULL, .size = 0},
            .error = ERR_INVALID_PARAMETER,
        };

    void *new = alloc->alloc(alloc, source.size);

    if (!new)
        return (ResultOwnedBuff){
            .value = {.bytes = NULL, .size = 0},
            .error = ERR_OUT_OF_MEMORY,
        };

    HeapBuff buff = (HeapBuff){.bytes = new, .size = source.size};
    buffer_copy_unsafe(source, buff);

    return (ResultOwnedBuff){
        .value = buff,
        .error = ERR_OK,
    };
}

/**
 * @brief Allocates and creates a copy of `source` on the heap.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 * May return NULL on allocation failure.
 *
 * ```c
 * ConstStr source = "Example string";
 * ConstBuff srcBuff = (ConstBuff){ .bytes = source, .size = string_size(source) + 1 };
 *
 * HeapBuff newBuff = buffer_dupe_noresult(&c_allocator, srcBuff);
 * if (!newBuff) // Error!
 * // newBuff.value.bytes == "Example string"
 * // newBuff.value.size == sizeof("Example string")
 * ```
 * @param alloc
 * @param source
 * @return HeapBuff
 */
HeapBuff buffer_dupe_noresult(Allocator *alloc, ConstBuff source)
{
    void *new = alloc->alloc(alloc, source.size);

    if (!new)
        return (HeapBuff){.bytes = NULL, .size = 0};

    HeapBuff buff = (HeapBuff){.bytes = new, .size = source.size};
    buffer_copy_unsafe(source, buff);
    return buff;
}

// TODO: buffer_resize
